// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerSubBlkSegmentsValid.h"
#include <exception>
#include <sstream>
#include <memory>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckSubBlkSegmentsValid::kDisplayName = "SubBlock-Segments in SubBlockDirectory are valid";
/*static*/const char* CCheckSubBlkSegmentsValid::kShortName = "subblksegmentsvalid";

CCheckSubBlkSegmentsValid::CCheckSubBlkSegmentsValid(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckSubBlkSegmentsValid::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckSubBlkSegmentsValid::kCheckType);

    this->reader_->EnumerateSubBlocks(
        [this](int index, const SubBlockInfo& info)->bool
        {
            try
            {
                this->reader_->ReadSubBlock(index);
            }
            catch (exception& exception)
            {
                CResultGatherer::Finding finding(CCheckSubBlkSegmentsValid::kCheckType);
                finding.severity = CResultGatherer::Severity::Fatal;
                stringstream ss;
                ss << "Error reading subblock #" << index;
                finding.information = ss.str();
                finding.details = exception.what();
                this->result_gatherer_.ReportFinding(finding);
            }

            return true;
        });

    this->result_gatherer_.FinishCheck(CCheckSubBlkSegmentsValid::kCheckType);
}
