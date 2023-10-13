// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <cstdint>
#include <map>
#include "cmdlineoptions.h"
#include "checks.h"

/// This class is intended to receive the findings from the individual checks. It is 
/// responsible for outputting them, and aggregating an overall result.
/// It relies on the semantic of:
/// - when a new checker starts executing, it calls into 'StartCheck'  
/// - when there is a finding to be reported, the checker calls into 'ReportFinding' (as  
///    many times as necessary)
/// - when a checker is done, it calls into 'FinishCheck'.  
/// Deviating from this semantic results in undefined behavior.
class CResultGatherer
{
private:
    struct CheckResult
    {
        CheckResult() : fatalMessagesCount(0), warningMessagesCount(0), infoMessagesCount(0) {}
        std::uint32_t fatalMessagesCount;
        std::uint32_t warningMessagesCount;
        std::uint32_t infoMessagesCount;

        std::uint32_t GetTotalMessagesCount() const { return this->fatalMessagesCount + this->warningMessagesCount + this->infoMessagesCount; }
    };

    std::map<CZIChecks, CheckResult> results_;

    const CCmdLineOptions& options_;
public:
    /// Values that represent the severity of the finding.
    enum class Severity
    {
        Fatal,			///< The finding is a fatal issue, i. e. the CZI-document is considered invalid and adverse behavior is expected.
        Warning,		///< The finding is a warning, i. e. a problem has been detected which may result in adverse behavior.
        Info			///< The finding is informational.
    };

    /// Values that represent the "aggregated result" of the complete run.
    enum class AggregatedResult
    {
        OK,					///< No warnings or fatal errors, only info.
        WithWarnings,		///< There have been one or more warnings, but not fatal error.
        ErrorsDetected		///< There have been one or more fatal errors.
    };

    struct Finding
    {
        explicit Finding(CZIChecks check) :check(check), severity(Severity::Info) {}
        CZIChecks   check;
        Severity    severity;
        std::string information;
        std::string details;
    };
public:
    explicit CResultGatherer(const CCmdLineOptions& options);
    void StartCheck(CZIChecks check);
    void ReportFinding(const Finding& finding);
    void FinishCheck(CZIChecks check);

    AggregatedResult GetAggregatedResult();
private:
    static void IncrementCounter(Severity severity, CheckResult& result);
};
