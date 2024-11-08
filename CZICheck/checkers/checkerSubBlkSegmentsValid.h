// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>

/// This checker is reading all the segments pointed to in the subblock-directory.
class CCheckSubBlkSegmentsValid : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::SubBlockDirectorySegmentValid;
    static const char* kDisplayName;
    static const char* kShortName; 

    CCheckSubBlkSegmentsValid(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
};
