// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <sstream>
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
    const ICheckerOptions& options_;
    std::ostringstream* output_stream_;  // If non-null, write JSON to this stream instead of console
    bool minified_;  // If true, output minified JSON instead of pretty-printed
    
    rapidjson::Document json_document_;
    rapidjson::Value test_results_;
    std::string current_checker_id;

public:
    explicit CResultGathererJson(const ICheckerOptions& options, std::ostringstream* output_stream = nullptr, bool minified = false);
    void StartCheck(CZIChecks check) override;
    void ReportFinding(const Finding& finding) override;
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
};

