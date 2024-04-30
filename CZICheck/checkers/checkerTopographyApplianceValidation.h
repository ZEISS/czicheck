// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"

class CCheckTopgraphyApplianceMetadata : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::ApplianceMetadataTopographyItemValid;
    static const char* kDisplayName;
    static const char* kShortName;
    
    CCheckTopgraphyApplianceMetadata(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        CResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
private:
    void CheckApplianceMetadataSection();
};
