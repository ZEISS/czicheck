// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerbase.h"
#include <memory>

using namespace std;
using namespace libCZI;

std::shared_ptr<libCZI::ICziMetadata> CCheckerBase::GetCziMetadataAndReportErrors(CZIChecks check)
{
    shared_ptr<IMetadataSegment>  metadata_segment;

    try
    {
        metadata_segment = this->reader_->ReadMetadataSegment();
    }
    catch (exception& ex)
    {
        IResultGatherer::Finding finding(check);
        finding.severity = IResultGatherer::Severity::Warning;
        finding.information = "Could not read metadata-segment";
        finding.details = ex.what();
        this->result_gatherer_.ReportFinding(finding);
    }

    if (metadata_segment)
    {
        shared_ptr<ICziMetadata> czi_metadata;
        try
        {
            czi_metadata = metadata_segment->CreateMetaFromMetadataSegment();
        }
        catch (exception& ex)
        {
            IResultGatherer::Finding finding(check);
            finding.severity = IResultGatherer::Severity::Fatal;
            finding.information = "Invalid metadata-segment";
            finding.details = ex.what();
            this->result_gatherer_.ReportFinding(finding);
        }

        if (czi_metadata)
        {
            if (!czi_metadata->IsXmlValid())
            {
                IResultGatherer::Finding finding(check);
                finding.severity = IResultGatherer::Severity::Fatal;
                finding.information = "The metadata is not well-formed XML";
                this->result_gatherer_.ReportFinding(finding);
            }
            else
            {
                return czi_metadata;
            }
        }
    }

    return nullptr;
}
