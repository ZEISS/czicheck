// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <cstdint>
#include <map>
#include "cmdlineoptions.h"
#include "checks.h"
#include "IResultGatherer.h"

// #include "rapidjson/document.h"
// #include "rapidjson/rapidjson.h"
// #include "rapidjson/writer.h"
// #include "rapidjson/stringbuffer.h"
// #include "rapidjson/prettywriter.h"


/// This class is intended to receive the findings from the individual checks. It is 
/// responsible for outputting them, and aggregating an overall result.
/// It relies on the semantic of:
/// - when a new checker starts executing, it calls into 'StartCheck'  
/// - when there is a finding to be reported, the checker calls into 'ReportFinding' (as  
///    many times as necessary)
/// - when a checker is done, it calls into 'FinishCheck'.  
/// Deviating from this semantic results in undefined behavior.
class CResultGatherer : public IResultGatherer
{
private:
    // std::map<CZIChecks, CheckResult> results_;
    const CCmdLineOptions& options_;
    // rapidjson::Document json_document_;
    // rapidjson::Value test_results_;
    // std::string current_checker_id;

public:
    explicit CResultGatherer(const CCmdLineOptions& options);
    void StartCheck(CZIChecks check) override;
    void StartCheckJson(CZIChecks check);
    void ReportFinding(const Finding& finding) override;
    void ReportFindingJson(const Finding& finding);
    void FinishCheck(CZIChecks check) override;
    void FinalizeChecks() override;
};
