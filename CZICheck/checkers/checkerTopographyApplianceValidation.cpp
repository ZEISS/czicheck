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

    this->result_gatherer_.FinishCheck(CCheckTopgraphyApplianceMetadata::kCheckType);
}

void CCheckTopgraphyApplianceMetadata::CheckApplianceMetadataSection()
{
}
