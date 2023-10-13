// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerSubBlkDirPositions.h"
#include <sstream>
#include <memory>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckSubBlkDirPositions::kDisplayName = "SubBlock-Segment in SubBlockDirectory within file";
/*static*/const char* CCheckSubBlkDirPositions::kShortName = "subblksegmentsinfile";

CCheckSubBlkDirPositions::CCheckSubBlkDirPositions(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckSubBlkDirPositions::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckSubBlkDirPositions::kCheckType);

    if (this->additional_info_.totalFileSize > 0)
    {
        this->reader_->EnumerateSubBlocksEx(
            [&](int index, const DirectorySubBlockInfo& info)->bool
            {
                // todo: include the minimal size of a segment
                if (info.filePosition >= this->additional_info_.totalFileSize)
                {
                    CResultGatherer::Finding finding(CCheckSubBlkDirPositions::kCheckType);
                    finding.severity = CResultGatherer::Severity::Fatal;
                    ostringstream string_stream;
                    string_stream << "position of subblock #" << index << " (=" << info.filePosition << ") is beyond filesize (=" << this->additional_info_.totalFileSize << ")";
                    finding.information = string_stream.str();
                    this->result_gatherer_.ReportFinding(finding);
                }

                return true;
            });
    }

    this->result_gatherer_.FinishCheck(CCheckSubBlkDirPositions::kCheckType);
}
