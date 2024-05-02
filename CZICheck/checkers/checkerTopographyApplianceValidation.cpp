// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerTopographyApplianceValidation.h"

using namespace std;
using namespace libCZI;

/*static*/const char* CCheckTopgraphyApplianceMetadata::kDisplayName = "Basic semantic checks for TopographyDataItems";
/*static*/const char* CCheckTopgraphyApplianceMetadata::kShortName = "topographymetadata";

CCheckTopgraphyApplianceMetadata::CCheckTopgraphyApplianceMetadata(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckTopgraphyApplianceMetadata::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckTopgraphyApplianceMetadata::kCheckType);

    const auto metadata_segment = this->GetCziMetadataAndReportErrors(CCheckTopgraphyApplianceMetadata::kCheckType);

    this->CheckTopographySectionExisting(metadata_segment);

    this->result_gatherer_.FinishCheck(CCheckTopgraphyApplianceMetadata::kCheckType);
}

void CCheckTopgraphyApplianceMetadata::CheckTopographySectionExisting(const std::shared_ptr<libCZI::ICziMetadata> czi_metadata)
{
    string appliance_path = "ImageDocument/Metadata/Appliances";
    auto metadata_node = czi_metadata->GetChildNodeReadonly(appliance_path.c_str());

    if (!metadata_node)
    {
        CResultGatherer::Finding finding(CCheckTopgraphyApplianceMetadata::kCheckType);
        finding.severity = CResultGatherer::Severity::Fatal;
        finding.information = "The ImageDocument does not contain Appliance metadata!";
        this->result_gatherer_.ReportFinding(finding);

        return;
    }

    appliance_path
        .append("/Appliance[Id=")
        .append(this->kTopographyItemId)
        .append("]");

    metadata_node = czi_metadata->GetChildNodeReadonly(appliance_path.c_str());

    if (!metadata_node)
    {
        CResultGatherer::Finding finding(CCheckTopgraphyApplianceMetadata::kCheckType);
        finding.severity = CResultGatherer::Severity::Fatal;
        finding.information = "The ImageDocument does not contain a Topography section in the metadata!";
        this->result_gatherer_.ReportFinding(finding);

        return;
    }
}
