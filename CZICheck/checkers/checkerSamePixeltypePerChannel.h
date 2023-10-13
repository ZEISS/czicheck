// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>

/// This checker checks whether the pixel types of all subblocks with the same
/// C-index is the same.
/// Pathologies:
/// - if there is no C-dimension, then we currently don't do any check
class CCheckSamePixeltypePerChannel : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::SamePixeltypePerChannel;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckSamePixeltypePerChannel(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        CResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
private:
    void CheckIfSamePixeltypeInChannel(int c);
};
