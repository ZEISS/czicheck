// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <memory>
#include "checkerbase.h"

/// This checker is testing whether the indices are consecutive.
class CCheckConsecutivePlaneIndices : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::PlaneIndicesAreConsecutive;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckConsecutivePlaneIndices(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGathererReport& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
private:
    void CheckForConsecutiveIndices();
};
