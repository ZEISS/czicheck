// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <memory>
#include <string>
#include "checkerbase.h"

class CCheckDuplicateCoordinates : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::DuplicateSubBlockCoordinates;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckDuplicateCoordinates(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGatherer& result_gatherer,
        const CheckerCreateInfo& additionalInfo);
    void RunCheck() override;
private:
    void CheckForDuplicates(std::vector<libCZI::SubBlockInfo>& subblock_infos);
    static std::string GetSubblockAsString(const libCZI::SubBlockInfo& subblock_info);
};
