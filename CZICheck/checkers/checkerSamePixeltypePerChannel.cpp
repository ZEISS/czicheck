// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerSamePixeltypePerChannel.h"

#include <algorithm>
#include <vector>
#include <memory>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckSamePixeltypePerChannel::kDisplayName = "check that the subblocks of a channel have the same pixeltype";
/*static*/const char* CCheckSamePixeltypePerChannel::kShortName = "samepixeltypeperchannel";

CCheckSamePixeltypePerChannel::CCheckSamePixeltypePerChannel(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckSamePixeltypePerChannel::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckSamePixeltypePerChannel::kCheckType);

    const auto statistics = this->reader_->GetStatistics();
    int start_c, size_c;

    // If there is no C-index with the statistics, this means that no C-dimension is used (with any subblock).
    // In this case, we don't do any check currently - we might want to check then whether all subblocks have
    //  the same pixel type.
    if (statistics.dimBounds.TryGetInterval(DimensionIndex::C, &start_c, &size_c))
    {
        for (int c = start_c; c < start_c + size_c; ++c)
        {
            this->CheckIfSamePixeltypeInChannel(c);
        }
    }

    this->result_gatherer_.FinishCheck(CCheckSamePixeltypePerChannel::kCheckType);
}

void CCheckSamePixeltypePerChannel::CheckIfSamePixeltypeInChannel(int c)
{
    CDimCoordinate plane_coordinate({ { DimensionIndex::C, c } });
    PixelType pixeltype{ PixelType::Invalid };
    this->reader_->EnumSubset(
        &plane_coordinate,
        nullptr,
        false,
        [&](int index, const SubBlockInfo& info)->bool
        {
            if (pixeltype != PixelType::Invalid)
            {
                if (info.pixelType != pixeltype)
                {
                    IResultGatherer::Finding finding(CCheckSamePixeltypePerChannel::kCheckType);
                    finding.severity = IResultGatherer::Severity::Warning;
                    stringstream ss;
                    ss << "pixeltype of subblock #" << index << " (" << Utils::PixelTypeToInformalString(info.pixelType) <<
                        ") differs from the pixeltype determined for channel " << c << " (" << Utils::PixelTypeToInformalString(pixeltype) << ")";
                    finding.information = ss.str();
                    this->result_gatherer_.ReportFinding(finding);
                }
            }
            else
            {
                pixeltype = info.pixelType;
            }

            return true;
        });
}
