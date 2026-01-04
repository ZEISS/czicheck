// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerConsistentCoordinates.h"
#include <memory>
#include <string>
#include <vector>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckConsistentCoordinates::kDisplayName = "check subblock's coordinates for 'consistent dimensions'";
/*static*/const char* CCheckConsistentCoordinates::kShortName = "subblkdimconsistent";

CCheckConsistentCoordinates::CCheckConsistentCoordinates(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckConsistentCoordinates::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckConsistentCoordinates::kCheckType);

    this->RunCheckDefaultExceptionHandling([this]()
        {
            vector<SubBlockInfo> subBlockInfos;
            subBlockInfos.reserve(this->reader_->GetStatistics().subBlockCount);

            this->reader_->EnumerateSubBlocks(
                [&](int index, const SubBlockInfo& info)->bool
                {
                    subBlockInfos.emplace_back(info);
                    return true;
                });

            this->CheckForSameDimensions(subBlockInfos);
        });

    this->result_gatherer_.FinishCheck(CCheckConsistentCoordinates::kCheckType);
}

void CCheckConsistentCoordinates::CheckForSameDimensions(const std::vector<libCZI::SubBlockInfo>& subblocks) const
{
    if (subblocks.size() <= 1)
    {
        return;
    }

    const auto& expected_dimensions = subblocks.front().coordinate;

    for (size_t i = 1; i < subblocks.size(); ++i)
    {
        const auto& info = subblocks[i];
        if (!Utils::HasSameDimensions(&info.coordinate, &expected_dimensions))
        {
            IResultGatherer::Finding finding(CCheckConsistentCoordinates::kCheckType);
            finding.check = CZIChecks::ConsistentSubBlockCoordinates;
            finding.severity = IResultGatherer::Severity::Fatal;
            stringstream ss;
            ss << "subblock #" << i << " has dimensions \"" << GetDimensionsAsInformalString(&info.coordinate)
                << "\", whereas \"" << GetDimensionsAsInformalString(&expected_dimensions) << "\" was expected.";
            finding.information = ss.str();
            this->ThrowIfFindingResultIsStop(this->result_gatherer_.ReportFinding(finding));
        }
    }
}

/*static*/std::string CCheckConsistentCoordinates::GetDimensionsAsInformalString(const libCZI::IDimCoordinate* coordinate)
{
    stringstream ss;
    bool is_first = true;
    for (int i = static_cast<int>(DimensionIndex::MinDim); i <= static_cast<int>(DimensionIndex::MaxDim); ++i)
    {
        int value;
        if (coordinate->TryGetPosition(static_cast<DimensionIndex>(i), &value))
        {
            if (is_first)
            {
                ss << Utils::DimensionToChar(static_cast<DimensionIndex>(i));
                is_first = false;
            }
            else
            {
                ss << ',' << Utils::DimensionToChar(static_cast<DimensionIndex>(i));
            }
        }
    }

    return ss.str();
}
