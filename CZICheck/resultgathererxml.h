// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IResultGatherer.h"
#include "cmdlineoptions.h"
#include "checks.h"

#include "pugixml.hpp"
#include "pugiconfig.hpp"
#include <string>

class CResultGathererXml : public IResultGatherer
{
private:
    const CCmdLineOptions& options_;
    std::string current_checker_id_;

    pugi::xml_document xml_document_;
    pugi::xml_node root_node_;
    pugi::xml_node test_node_;

public:
    explicit CResultGathererXml(const CCmdLineOptions& options);
    void StartCheck(CZIChecks check) override;
    void ReportFinding(const Finding& finding) override;
    void FinishCheck(CZIChecks check) override;
    void FinalizeChecks() override;
private:
    static std::wstring ConvertToWideString(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }
};

