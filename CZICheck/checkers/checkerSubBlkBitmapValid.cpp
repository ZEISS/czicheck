// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerSubBlkBitmapValid.h"
#include <exception>
#include <sstream>
#include <memory>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckSubBlkBitmapValid::kDisplayName = "SubBlock-Segments in SubBlockDirectory are valid and valid content";
/*static*/const char* CCheckSubBlkBitmapValid::kShortName = "subblkbitmapvalid";

CCheckSubBlkBitmapValid::CCheckSubBlkBitmapValid(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckSubBlkBitmapValid::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckSubBlkBitmapValid::kCheckType);

    this->reader_->EnumerateSubBlocks(
        [this](int index, const SubBlockInfo& info)->bool
        {
            try
            {
                auto sub_block = this->reader_->ReadSubBlock(index);
                const auto compression_mode = sub_block->GetSubBlockInfo().GetCompressionMode();
                if (compression_mode != CompressionMode::Invalid)
                {
                    // According to documentation, for a subblock with a compression mode which is *not* supported by
                    //  libCZI, we'd be getting CompressionMode::Invalid here. So, if we get a valid compression mode,
                    //  then we can rightfully expect that the subblock can be decoded, or that we can get a bitmap here
                    try
                    {
                        auto bitmap = sub_block->CreateBitmap();
                    }
                    catch (exception& exception)
                    {
                        IResultGatherer::Finding finding(CCheckSubBlkBitmapValid::kCheckType);
                        finding.severity = IResultGatherer::Severity::Fatal;
                        stringstream ss;
                        ss << "Error decoding subblock #" << index << " with compression \"" << Utils::CompressionModeToInformalString(compression_mode) << "\"";
                        finding.information = ss.str();
                        finding.details = exception.what();
                        this->result_gatherer_.ReportFinding(finding);
                    }
                }
                else
                {
                    IResultGatherer::Finding finding(CCheckSubBlkBitmapValid::kCheckType);
                    finding.severity = IResultGatherer::Severity::Info;
                    stringstream ss;
                    ss << "Subblock #" << index << " has a non-standard compression mode (" << sub_block->GetSubBlockInfo().compressionModeRaw << ")";
                    finding.information = ss.str();
                    this->result_gatherer_.ReportFinding(finding);
                }
            }
            catch (exception& exception)
            {
                IResultGatherer::Finding finding(CCheckSubBlkBitmapValid::kCheckType);
                finding.severity = IResultGatherer::Severity::Fatal;
                stringstream ss;
                ss << "Error reading subblock #" << index;
                finding.information = ss.str();
                finding.details = exception.what();
                this->result_gatherer_.ReportFinding(finding);
            }

            return true;
        });

    this->result_gatherer_.FinishCheck(CCheckSubBlkBitmapValid::kCheckType);
}

