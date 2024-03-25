// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>

/// This checker reads all the segments pointed to in the subblock-directory
/// and decodes the subblock-content.
class CCheckSubBlkBitmapValid : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::CheckSubBlockBitmapValid;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckSubBlkBitmapValid(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        CResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
};
