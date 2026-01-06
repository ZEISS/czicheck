// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "cmdlineoptions.h"
#include "IResultGatherer.h"
#include <optional>

class ResultGathererBase
{
private:
    const CCmdLineOptions& options_;
    std::optional<CZIChecks> current_checker_;
    std::map<CZIChecks, IResultGatherer::CheckResult> results_;

public:
    explicit ResultGathererBase(const CCmdLineOptions& options);

protected:
    void CoreStartCheck(CZIChecks check);
    void CoreReportFinding(const IResultGatherer::Finding& finding);
    void CoreFinishCheck(CZIChecks check) ;

    IResultGatherer::CheckResult CoreGetAggregatedCounts() const;
    IResultGatherer::CheckResult GetCheckResultForCurrentlyActiveChecker() const;
    const std::shared_ptr<ILog>& GetLog() const { return this->options_.GetLog(); }
    int GetMaxNumberOfMessagesToPrint() const { return this->options_.GetMaxNumberOfMessagesToPrint(); }
    bool GetPrintDetailsOfMessages() const { return this->options_.GetPrintDetailsOfMessages(); }

    /// \brief Determines whether processing should continue or stop after reporting a finding.
    ///
    /// This method evaluates the severity of a finding and the configured fail-fast mode
    /// to decide if the checker execution should be stopped immediately or continue processing.
    ///
    /// \param finding The finding to evaluate, containing severity and other diagnostic information.
    ///
    /// \return ReportFindingResult::Stop if the finding is fatal and fail-fast mode is enabled
    ///         for fatal errors (either overall or per-checker); ReportFindingResult::Continue otherwise.
    IResultGatherer::ReportFindingResult DetermineReportFindingResult(const IResultGatherer::Finding& finding) const;

    /// \brief Converts a finding's severity into a short string representation for logs and reports.
    ///
    /// \param finding The finding whose severity should be converted to text.
    ///
    /// \return A null-terminated string identifying the severity (e.g., "INFO", "WARNING", "FATAL");
    ///         the string has static storage duration and must not be freed.
    static const char* FindingSeverityToString(const IResultGathererReport::Finding& finding);
};
