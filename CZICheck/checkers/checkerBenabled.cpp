// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerBenabled.h"

#include <algorithm>
#include <memory>
#include <vector>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckBenabled::kDisplayName = "check whether the document uses the deprecated 'B-index'";
/*static*/const char* CCheckBenabled::kShortName = "benabled";

CCheckBenabled::CCheckBenabled(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
        CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckBenabled::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckBenabled::kCheckType);

    auto statistics = this->reader_->GetStatistics();
    int start_b, size_b;
    if (statistics.dimBounds.TryGetInterval(DimensionIndex::B, &start_b, &size_b))
    {
        if (size_b > 1)
        {
            CResultGatherer::Finding finding(CCheckBenabled::kCheckType);
            finding.severity = CResultGatherer::Severity::Warning;
            stringstream ss;
            ss << "document contains deprecated B-dimension (sizeB=" << size_b << ")";
            finding.information = ss.str();
            this->result_gatherer_.ReportFinding(finding);
        }
        else
        {
            CResultGatherer::Finding finding(CCheckBenabled::kCheckType);
            finding.severity = CResultGatherer::Severity::Info;
            stringstream ss;
            ss << "coordinates contain deprecated B-dimension (sizeB=" << size_b << ")";
            finding.information = ss.str();
            this->result_gatherer_.ReportFinding(finding);
        }
    }

    this->result_gatherer_.FinishCheck(CCheckBenabled::kCheckType);
}
