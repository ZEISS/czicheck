// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>
#include <vector>
#include <string>

class CCheckConsistentCoordinates : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::ConsistentSubBlockCoordinates;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckConsistentCoordinates(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;

private:
    void CheckForSameDimensions(const std::vector<libCZI::SubBlockInfo>& subblocks) const;
    static std::string GetDimensionsAsInformalString(const libCZI::IDimCoordinate* coordinate);
};
