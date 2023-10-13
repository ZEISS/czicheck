// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerDuplicateCoordinates.h"

#include <algorithm>
#include <vector>
#include <memory>
#include <string>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckDuplicateCoordinates::kDisplayName = "check subblock's coordinates being unique";
/*static*/const char* CCheckDuplicateCoordinates::kShortName = "subblkcoordsunique";

CCheckDuplicateCoordinates::CCheckDuplicateCoordinates(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckDuplicateCoordinates::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckDuplicateCoordinates::kCheckType);

    vector<SubBlockInfo> subblock_infos;
    subblock_infos.reserve(this->reader_->GetStatistics().subBlockCount);

    this->reader_->EnumerateSubBlocks(
        [&](int index, const SubBlockInfo& info)->bool
        {
            subblock_infos.emplace_back(info);
            return true;
        });


    this->CheckForDuplicates(subblock_infos);

    this->result_gatherer_.FinishCheck(CCheckDuplicateCoordinates::kCheckType);
}

void CCheckDuplicateCoordinates::CheckForDuplicates(vector<SubBlockInfo>& subblock_infos)
{
    vector<int> indices_sorted;
    indices_sorted.reserve(subblock_infos.size());
    for (int i = 0; i < subblock_infos.size(); ++i)
    {
        indices_sorted.emplace_back(i);
    }

    struct Comparer
    {
    private:
        const vector<SubBlockInfo>& subBlockInfos;
    public:
        explicit Comparer(const vector<SubBlockInfo>& subBlockInfos) : subBlockInfos(subBlockInfos) {}

        bool operator() (int a, int b) const
        {
            return Compare(subBlockInfos[a], subBlockInfos[b]);
        }
    private:
        /// Compares two const SubBlockInfo objects to determine their relative ordering. The value
        /// returned indicates whether the first element is considered to go before the second in
        /// _strict weak ordering_ semantic.
        /// \param  a The first element to be compared.
        /// \param  b The second element to be compared.
        /// \returns True if the first element is considered to go before the second.
        static bool Compare(const SubBlockInfo& a, const SubBlockInfo& b)
        {
            // first criterion : the zoom (or the pyramid layer)
            if (a.logicalRect.w == a.physicalSize.w && a.logicalRect.h == a.physicalSize.h &&
                b.logicalRect.w == b.physicalSize.w && b.logicalRect.h == b.physicalSize.h)
            {
                if (a.GetZoom() > b.GetZoom())
                {
                    return true;
                }
                else if (a.GetZoom() < b.GetZoom())
                {
                    return false;
                }
            }

            const int r = Utils::Compare(&a.coordinate, &b.coordinate);
            if (r > 0)
            {
                return true;
            }
            else if (r < 0)
            {
                return false;
            }

            if (a.IsMindexValid() && b.IsMindexValid())
            {
                if (a.mIndex > b.mIndex)
                {
                    return true;
                }
                else if (a.mIndex < b.mIndex)
                {
                    return false;
                }
            }
            else if (a.IsMindexValid() && !b.IsMindexValid())
            {
                return true;
            }

            return false;
        }
    };

    const Comparer comparer(subblock_infos);
    sort(indices_sorted.begin(), indices_sorted.end(), comparer);

    struct EqualityComparer
    {
    private:
        const vector<SubBlockInfo>& subBlockInfos;
    public:
        explicit EqualityComparer(const vector<SubBlockInfo>& subblock_infos) : subBlockInfos(subblock_infos) {}

        bool operator() (int a, int b) const
        {
            return Compare(subBlockInfos[a], subBlockInfos[b]);
        }
    private:
        [[nodiscard]] bool Compare(const SubBlockInfo& a, const SubBlockInfo& b) const
        {
            int r = Utils::Compare(&a.coordinate, &b.coordinate);
            if (r == 0)
            {
                if (a.IsMindexValid() && b.IsMindexValid())
                {
                    if (a.mIndex != b.mIndex)
                    {
                        return false;
                    }
                    else
                    {
                        // if both contain M-index, and if their M-index is equal, then we have a problem
                        return true;
                    }
                }
                else if (a.IsMindexValid() != b.IsMindexValid())
                {
                    // if one has a valid m-index and the other not - let's consider them as "not equal"
                    return false;
                }

                // if we get here - both do not have an m-index

                // if they are not at the same position - consider them different
                if (a.logicalRect.x != b.logicalRect.x || a.logicalRect.y != b.logicalRect.y)
                {
                    return false;
                }

                // if the subblocks are on a different pyramid-layer - then consider them not equal
                if (abs(a.GetZoom() - b.GetZoom()) > 1.0 / 1024)
                {
                    // this condition is rather makeshift
                    return false;
                }

                return true;
            }

            return false;
        }
    };

    const EqualityComparer equality_comparer(subblock_infos);

    const auto it = std::adjacent_find(indices_sorted.begin(), indices_sorted.end(), equality_comparer);
    if (it != indices_sorted.end())
    {
        CResultGatherer::Finding finding(CCheckDuplicateCoordinates::kCheckType);
        finding.severity = CResultGatherer::Severity::Fatal;
        stringstream ss;
        ss << "duplicate subblock #" << *it << " and # " << *(it + 1) << " : \"" << GetSubblockAsString(subblock_infos[*it]) << "\"";
        finding.information = ss.str();
        this->result_gatherer_.ReportFinding(finding);
    }
}

/*static*/std::string CCheckDuplicateCoordinates::GetSubblockAsString(const SubBlockInfo& subblock_info)
{
    auto coordinate_as_string = Utils::DimCoordinateToString(&subblock_info.coordinate);
    stringstream ss;
    ss << coordinate_as_string;
    if (subblock_info.IsMindexValid())
    {
        ss << " M=" << subblock_info.mIndex;
    }

    return ss.str();
}
