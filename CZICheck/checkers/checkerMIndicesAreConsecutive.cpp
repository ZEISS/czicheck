// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerMIndicesAreConsecutive.h"
#include "utils/planeenumerator.h"
#include "utils/utils.h"
#include <algorithm>
#include <sstream>

/*static*/const char* CCheckMIndicesAreConsecutive::kDisplayName = "check if M indices are consecutive";
/*static*/const char* CCheckMIndicesAreConsecutive::kShortName = "mindicesconsecutive";

CCheckMIndicesAreConsecutive::CCheckMIndicesAreConsecutive(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGathererReport& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckMIndicesAreConsecutive::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckMIndicesAreConsecutive::kCheckType);

    this->RunCheckDefaultExceptionHandling([this]()
        {
            const auto statistics = this->reader_->GetStatistics();

            PlaneEnumerator plane_enumerator{ statistics };

            // iterate over all planes
            for (const auto& plane_info : plane_enumerator)
            {
                this->CheckMIndicesAreConsecutiveForPlane(plane_info.plane_coordinate);
            }
        });

    this->result_gatherer_.FinishCheck(CCheckMIndicesAreConsecutive::kCheckType);
}

void CCheckMIndicesAreConsecutive::CheckMIndicesAreConsecutiveForPlane(const libCZI::CDimCoordinate& plane_coordinate)
{
    auto result = this->GetSubBlocksForPlane(plane_coordinate);
    if (!result.has_value())
    {
        // One or more subblocks on this plane have no valid M-index; report and bail out —
        // consecutive M-index checking is only meaningful when all M-indices are present.
        IResultGatherer::Finding finding(CCheckMIndicesAreConsecutive::kCheckType);
        finding.severity = IResultGatherer::Severity::Warning;
        std::ostringstream ss;
        ss << "One or more subblocks on plane [" << libCZI::Utils::DimCoordinateToString(&plane_coordinate)
            << "] have no valid M-index; cannot check M-index consecutivity for this plane.";
        finding.information = ss.str();
        this->ThrowIfFindingResultIsStop(this->result_gatherer_.ReportFinding(finding));
        return;
    }

    auto& subblocks_on_plane = result.value();
    if (subblocks_on_plane.empty())
    {
        return;
    }

    // Sort by m_index — O(n log n), then a single O(n) pass to detect issues.
    std::sort(
        subblocks_on_plane.begin(),
        subblocks_on_plane.end(),
        [](const SubBlockIndexAndMIndex& a, const SubBlockIndexAndMIndex& b) 
        { 
            return a.m_index < b.m_index; 
        });

    // Must start at 0.
    if (subblocks_on_plane.front().m_index != 0)
    {
        IResultGatherer::Finding finding(CCheckMIndicesAreConsecutive::kCheckType);
        finding.severity = IResultGatherer::Severity::Warning;
        std::ostringstream ss;
        ss << "M indices do not start at 0 for plane [" << libCZI::Utils::DimCoordinateToString(&plane_coordinate)
            << "]: smallest M index is " << subblocks_on_plane.front().m_index << ".";
        finding.information = ss.str();
        this->ThrowIfFindingResultIsStop(this->result_gatherer_.ReportFinding(finding));
        return;
    }

    // Each successive m_index must increment by exactly 1 (no gaps, no duplicates).
    for (std::size_t i = 1; i < subblocks_on_plane.size(); ++i)
    {
        const int expected = subblocks_on_plane[i - 1].m_index + 1;
        const int actual   = subblocks_on_plane[i].m_index;
        if (actual != expected)
        {
            IResultGatherer::Finding finding(CCheckMIndicesAreConsecutive::kCheckType);
            finding.severity = IResultGatherer::Severity::Warning;
            std::ostringstream ss;
            ss << "M indices are not consecutive for plane [" << libCZI::Utils::DimCoordinateToString(&plane_coordinate)
                << "]: expected " << expected << " but found " << actual
                << " (subblock index " << subblocks_on_plane[i].subblock_index << ").";
            finding.information = ss.str();
            this->ThrowIfFindingResultIsStop(this->result_gatherer_.ReportFinding(finding));
            return;
        }
    }
}

std::optional<std::vector<CCheckMIndicesAreConsecutive::SubBlockIndexAndMIndex>> CCheckMIndicesAreConsecutive::GetSubBlocksForPlane(const libCZI::CDimCoordinate& plane_coordinate)
{
    std::vector<SubBlockIndexAndMIndex> m_indices;
    bool has_invalid_m_index = false;
    this->reader_->EnumerateSubBlocks(
        [&](int index, const libCZI::SubBlockInfo& info)->bool
        {
            if (info.logicalRect.w == info.physicalSize.w && info.logicalRect.h == info.physicalSize.h &&
                Utils::CompareValidDimensions(plane_coordinate, info.coordinate))
            {
                if (!libCZI::Utils::IsValidMindex(info.mIndex))
                {
                    has_invalid_m_index = true;
                    return false; // stop enumeration early
                }

                m_indices.push_back({index, info.mIndex});
            }

            return true;
        });

    if (has_invalid_m_index)
    {
        return std::nullopt;
    }

    return m_indices;
}
