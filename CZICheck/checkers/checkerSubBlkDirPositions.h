// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>

/// This checker checks whether the subblock's file-position (retrieved from the subblock-directory)
/// are within the file. Note that only the position itself is checked, not content of the file at
/// this location.
/// For HTTP/HTTPS streams, the file size is determined by probing reads at various offsets using
/// binary search, which may incur some network overhead but enables validation.
/// Pathologies:
/// - if the filesize cannot be determined (e.g., stream larger than probe limit or errors during probing),
///   an info message is reported and no validation is performed
class CCheckSubBlkDirPositions : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::SubBlockDirectoryPositionsWithinRange;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckSubBlkDirPositions(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
};
