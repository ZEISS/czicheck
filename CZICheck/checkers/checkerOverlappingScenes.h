// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <memory>
#include <map>
#include "checkerbase.h"

/// This checker is about checking whether scenes are overlapping (on pyramid-layer 0).
class CCheckOverlappingScenesOnLayer0 : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::CCheckOverlappingScenesOnLayer0;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckOverlappingScenesOnLayer0(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        CResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
private:
    struct ScenePair
    {
        int sceneIndex1;
        int sceneIndex2;
    };

    struct SubBlockPair
    {
        int subBlockInFirstScene;
        int subBlockInSecondScene;
    };

    /// Check the scene-bounding-rectangles (for pyramid-layer-0) for overlap. If an overlap is found, it is reported
    /// to the specified functor (if it is non-null). If the functor returns false, then the operation is ended; otherwise
    /// additional overlaps are searched for. The return value indicates whether at least one overlap was found.
    ///
    /// \param  bounding_rectangles        The set of bounding rectangles.
    /// \param  add_overlapping_scene_pair Functor where overlapping rectangles are reported.
    ///
    /// \returns    True if there was at least one overlap found; false otherwise.
    static bool AreSceneBoundingRectanglesOverlapping(
        const std::map<int, libCZI::BoundingBoxes>& bounding_rectangles,
        const std::function<bool(int, int)>& add_overlapping_scene_pair);

    /// Check for overlapping subblocks in different scenes. The vector "overlappingScenePairs"
    /// specifies which scene-bounding-rectangles overlap, and subblocks in those scenes are
    /// checked here.
    ///
    /// \param  statistics                The statistics of the document.
    /// \param  overlapping_scene_pairs   Vector with pairs of scene-indices which are to be checked.
    void CheckForOverlappingSubblocksInDifferentScenes(
        const libCZI::SubBlockStatistics& statistics,
        const std::vector<ScenePair>& overlapping_scene_pairs);

    /// Check for overlapping subblocks within the specified plane and within the specified scenes.
    /// This method will also report a finding (in case it detected an overlap).
    ///
    /// \param  plane_coordinate The plane coordinate.
    /// \param  pair             The pair of scene indices to check.
    void CheckForOverlappingSubblocksInPlaneAndBetweenTwoScenes(const libCZI::IDimCoordinate* plane_coordinate, const ScenePair& pair);

    /// Determine if any pair of subblocks from the two vectors (containing subblock indices)
    /// are overlapping. If an overlapping pair is found, the pair is reported to the specified
    /// functor. If the functor returns false, the search is immediately cancelled.
    ///
    /// \param  subblocks_of_plane_in_first_scene    The subblocks in first scene (to be checked for overlap with the other set of subblocks).
    /// \param  subblocks_of_plane_in_second_scene   The subblocks in second scene (to be checked for overlap with the other set of subblocks).
    /// \param  report_overlapping_subblocks         If non-null, this functor will be called for every overlap detected. The return
    ///                                              value decides whether the search is continued or cancelled.
    ///
    /// \returns    True if at least one overlapping pair was found; false otherwise.
    bool AreOverlapping(
        const std::vector<int>& subblocks_of_plane_in_first_scene, 
        const std::vector<int>& subblocks_of_plane_in_second_scene,
        const std::function<bool(int, int)>& report_overlapping_subblocks) const;
};
