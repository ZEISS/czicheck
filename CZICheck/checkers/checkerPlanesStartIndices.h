// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <memory>
#include "checkerbase.h"

/// This checker is about checking whether the coordinates for the plane-coordinates start at index 0.
class CCheckPlanesStartIndices : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::PlanesIndicesStartAtZero;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckPlanesStartIndices(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGathererReport& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;
};
