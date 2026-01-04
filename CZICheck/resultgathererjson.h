// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include "IResultGatherer.h"
#include "resultgathererbase.h"
#include "cmdlineoptions.h"
#include "checks.h"

#include "rapidjson/document.h"

class CResultGathererJson : public IResultGatherer, ResultGathererBase
{
private:
    rapidjson::Document json_document_;
    rapidjson::Value test_results_;
    std::string current_checker_id;

public:
    explicit CResultGathererJson(const CCmdLineOptions& options);
    void StartCheck(CZIChecks check) override;
    ReportFindingResult ReportFinding(const Finding& finding) override;
    void FinishCheck(CZIChecks check) override;
    void FinalizeChecks() override;

private:
    static constexpr const char* kTestNameId = "name";
    static constexpr const char* kTestContainerId = "tests";
    static constexpr const char* kTestDescriptionId = "description";
    static constexpr const char* kTestResultId = "result";
    static constexpr const char* kTestFindingsId = "findings";
    static constexpr const char* kTestSeverityId = "severity";
    static constexpr const char* kTestDetailsId = "details";
    static constexpr const char* kTestAggregationId = "aggregatedresult";
    static constexpr const char* kTestFailFastId = "fail_fast_stopped";
};
