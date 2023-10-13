// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerOverlappingScenes.h"

#include <algorithm>
#include <vector>
#include <memory>
#include <map>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckOverlappingScenesOnLayer0::kDisplayName = "check if subblocks at pyramid-layer 0 of different scenes are overlapping";
/*static*/const char* CCheckOverlappingScenesOnLayer0::kShortName = "overlappingscenes";

CCheckOverlappingScenesOnLayer0::CCheckOverlappingScenesOnLayer0(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckOverlappingScenesOnLayer0::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckOverlappingScenesOnLayer0::kCheckType);

    const auto subblock_statistics = this->reader_->GetStatistics();

    // if there is no S-index, then we have nothing to do here
    if (subblock_statistics.dimBounds.IsValid(DimensionIndex::S))
    {
        vector<ScenePair> overlapping_scenes;

        // so, now we first check whether the "scene-minimal-bounding-rectangles" are overlapping,
        // and in "overlappingScenes" we will store which scenes are overlapping (as determined by
        // checking the bounding-rectangles)
        if (CCheckOverlappingScenesOnLayer0::AreSceneBoundingRectanglesOverlapping(
            subblock_statistics.sceneBoundingBoxes,
            [&](int scene_index1, int scene_index2)->bool
            {
                overlapping_scenes.emplace_back(ScenePair{ scene_index1, scene_index2 });
                return true;
            }))
        {
            // if there are overlaps found from checking the bounding-rectangles, we next check respective
            // case more detailed - i.e. by checking the subblocks in question itself
            this->CheckForOverlappingSubblocksInDifferentScenes(subblock_statistics, overlapping_scenes);
        }
    }

    this->result_gatherer_.FinishCheck(CCheckOverlappingScenesOnLayer0::kCheckType);
}

/*static*/bool CCheckOverlappingScenesOnLayer0::AreSceneBoundingRectanglesOverlapping(const std::map<int, BoundingBoxes>& bounding_rectangles, const std::function<bool(int, int)>& add_overlapping_scene_pair)
{
    bool overlapping_scenes_found = false;
    auto it = bounding_rectangles.cbegin();
    for (; std::distance(it, bounding_rectangles.cend()) > 1; ++it)
    {
        for (auto iteratorToCompareWith = next(it, 1); iteratorToCompareWith != bounding_rectangles.cend(); ++iteratorToCompareWith)
        {
            if (it->second.boundingBoxLayer0.IntersectsWith(iteratorToCompareWith->second.boundingBoxLayer0))
            {
                overlapping_scenes_found = true;
                if (add_overlapping_scene_pair)
                {
                    if (!add_overlapping_scene_pair(it->first, iteratorToCompareWith->first))
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }

    return overlapping_scenes_found;
}

void CCheckOverlappingScenesOnLayer0::CheckForOverlappingSubblocksInDifferentScenes(
    const libCZI::SubBlockStatistics& statistics,
    const std::vector<ScenePair>& overlapping_scene_pairs)
{
    // copy the "dimension bounds" from the statistics, and clear the "S-dimension" in it
    CDimBounds bounds_to_enumerate_planes(&statistics.dimBounds);
    bounds_to_enumerate_planes.Clear(DimensionIndex::S);

    // so, now enumerate all "plane-coordinates" within the "dim-bounds" of the document
    Utils::EnumAllCoordinates(
        bounds_to_enumerate_planes,
        [&](std::uint64_t no, const libCZI::CDimCoordinate& coordinate)->bool
        {
            // and now, for each plane, iterate for every scene-pair (for which there is an overlap of
            //  the bounding-rectangles)
            for (const auto& pair : overlapping_scene_pairs)
            {
                this->CheckForOverlappingSubblocksInPlaneAndBetweenTwoScenes(&coordinate, pair);
            }

            return true;
        });
}

void CCheckOverlappingScenesOnLayer0::CheckForOverlappingSubblocksInPlaneAndBetweenTwoScenes(const IDimCoordinate* plane_coordinate, const ScenePair& pair)
{
    std::vector<int> subblocks_of_plane_in_first_scene;
    std::vector<int> subblocks_of_plane_in_second_scene;

    // now, enumerate all subblocks within the specified plane, and then store the subblock-indices in to
    //  vectors - one for the subblocks in the scene "pair.sceneIndex1" and one for "pair.sceneIndex2"
    this->reader_->EnumSubset(
        plane_coordinate,
        nullptr,
        true,
        [&](int index, const SubBlockInfo& info)->bool
        {
            int s;
            if (info.coordinate.TryGetPosition(DimensionIndex::S, &s))
            {
                if (s == pair.sceneIndex1)
                {
                    subblocks_of_plane_in_first_scene.emplace_back(index);
                }
                else if (s == pair.sceneIndex2)
                {
                    subblocks_of_plane_in_second_scene.emplace_back(index);
                }
            }

            return true;
        });

    // and, now check whether there is an overlap of subblocks in the one list with the other
    vector< SubBlockPair> overlapping_pairs;
    bool are_overlapping = this->AreOverlapping(
        subblocks_of_plane_in_first_scene,
        subblocks_of_plane_in_second_scene,
        [&](int sbBlkIndexFirstScene, int sbBlkIndexSecondScene) -> bool
        {
            overlapping_pairs.emplace_back(SubBlockPair{ sbBlkIndexFirstScene, sbBlkIndexSecondScene });
            return true;
        });

    if (are_overlapping)
    {
        CResultGatherer::Finding finding(CCheckOverlappingScenesOnLayer0::kCheckType);
        finding.severity = CResultGatherer::Severity::Warning;
        stringstream ss;
        ss << "in plane " << Utils::DimCoordinateToString(plane_coordinate) << " there are overlapping subblocks in scene " << pair.sceneIndex1 << " and scene " << pair.sceneIndex2;
        finding.information = ss.str();

        ss = stringstream();
        ss << "The following subblocks overlap (1st is scene#" << pair.sceneIndex1 << ", 2nd is scene#" << pair.sceneIndex2 << "): ";
        bool is_first = true;
        for (const auto& overlappingPair : overlapping_pairs)
        {
            if (!is_first)
            {
                ss << ", ";
            }

            ss << "(" << overlappingPair.subBlockInFirstScene << "<->" << overlappingPair.subBlockInSecondScene << ")";
            is_first = false;
        }

        finding.details = ss.str();
        this->result_gatherer_.ReportFinding(finding);
    }
}

bool CCheckOverlappingScenesOnLayer0::AreOverlapping(
    const std::vector<int>& subblocks_of_plane_in_first_scene, 
    const std::vector<int>& subblocks_of_plane_in_second_scene, 
    const std::function<bool(int, int)>& report_overlapping_subblocks) const
{
    bool overlapping_subblocks_found = false;

    // now, check if any subblock in "subBlocksOfPlaneInFirstScene" is overlapping with any subblock in "subBlocksOfPlaneInSecondScene"
    for (const auto subblock_index_first_scene : subblocks_of_plane_in_first_scene)
    {
        SubBlockInfo subblockInfo1;
        this->reader_->TryGetSubBlockInfo(subblock_index_first_scene, &subblockInfo1);

        for (const auto subblockIndexSecondScene : subblocks_of_plane_in_second_scene)
        {
            SubBlockInfo subblock_info2;
            this->reader_->TryGetSubBlockInfo(subblockIndexSecondScene, &subblock_info2);
            if (subblockInfo1.logicalRect.IntersectsWith(subblock_info2.logicalRect))
            {
                overlapping_subblocks_found = true;
                if (report_overlapping_subblocks)
                {
                    if (!report_overlapping_subblocks(subblock_index_first_scene, subblockIndexSecondScene))
                    {
                        break;
                    }
                }
            }
        }
    }

    return overlapping_subblocks_found;
}
