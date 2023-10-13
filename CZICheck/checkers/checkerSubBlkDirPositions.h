// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>

/// This checker checks whether the subblock's file-position (retrieved from the subblock-directory)
/// are within the file. Note that only the position itself is checked, not content of the file at
/// this location.
/// Pathologies:
/// - if the filesize is unknown, then this test does nothing
class CCheckSubBlkDirPositions : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::SubBlockDirectoryPositionsWithinRange;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckSubBlkDirPositions(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        CResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
};
