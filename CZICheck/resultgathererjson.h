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
    CheckResult GetAggregatedCounts() const override;

private:
    static const char* kTestNameId;
    static const char* kTestContainerId;
    static const char* kTestDescriptionId;
    static const char* kTestResultId;
    static const char* kTestFindingsId;
    static const char* kTestSeverityId;
    static const char* kTestDetailsId;
    static const char* kTestAggregationId;
    static const char* kTestFailFastId;
};
