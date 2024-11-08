// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IResultGatherer.h"
#include "cmdlineoptions.h"
#include "checks.h"

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

class CResultGathererJson : public IResultGatherer
{
  private:
    const CCmdLineOptions& options_;
    
    rapidjson::Document json_document_;
    rapidjson::Value test_results_;
    std::string current_checker_id;

  public:
    explicit CResultGathererJson(const CCmdLineOptions& options);
    void StartCheck(CZIChecks check) override;
    void ReportFinding(const Finding& finding) override;
    void FinishCheck(CZIChecks check) override;
    void FinalizeChecks() override;
};