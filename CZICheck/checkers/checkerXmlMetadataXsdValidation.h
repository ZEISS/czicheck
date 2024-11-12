// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <CZICheck_Config.h>

#if CZICHECK_XERCESC_AVAILABLE
#include <memory>
#include <string>

#include "checkerbase.h"

/// This checker is validating the XML-metadata against the XSD-schema.
class CCheckXmlMetadataXsdValidation : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::XmlMetadataSchemaValidation;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckXmlMetadataXsdValidation(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
private:
    std::string GetCziMetadataXml();
};

#endif
