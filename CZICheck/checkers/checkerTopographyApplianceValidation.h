// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"

class CCheckTopgraphyApplianceMetadata : public IChecker, CCheckerBase
{
private:
    static constexpr const char* kTopographyItemId = "Topography:1";

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
    void CheckTopographySectionExisting(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata);
};
