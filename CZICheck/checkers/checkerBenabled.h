// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>

/// This checker is testing whether the (deprecated) B-dimension is used.
class CCheckBenabled : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::BenabledDocument;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckBenabled(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        CResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
};
