// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerMissingMindex.h"

#include <algorithm>
#include <vector>
#include <memory>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckMissingMindex::kDisplayName = "check if all subblocks have the M index";
/*static*/const char* CCheckMissingMindex::kShortName = "minallsubblks";

CCheckMissingMindex::CCheckMissingMindex(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckMissingMindex::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckMissingMindex::kCheckType);

    this->RunCheckDefaultExceptionHandling([this]()
        {
            int count = 0;

            /// Enumerates all subblocks on layer 0 (i.e. non-pyramid subblocks)
            /// and simply checks for IsMindexValid.
            this->reader_->EnumSubset(
                nullptr,
                nullptr,
                true,
                [&](int index, const SubBlockInfo& info)->bool
                {
                        // TODO(JBL): we might want to allow for missing M indices if the image is not a mosaic.
                        if (!info.IsMindexValid())
                        {
                            count++;
                        }

                        return true;
                });

            if (count > 0)
            {
                IResultGatherer::Finding finding(CCheckMissingMindex::kCheckType);
                finding.severity = IResultGatherer::Severity::Warning;
                stringstream ss;
                ss << "There are " << count << " subblocks with no M index.";
                finding.information = ss.str();
                this->ThrowIfFindingResultIsStop(this->result_gatherer_.ReportFinding(finding));
            }
        });

    this->result_gatherer_.FinishCheck(CCheckMissingMindex::kCheckType);
}
