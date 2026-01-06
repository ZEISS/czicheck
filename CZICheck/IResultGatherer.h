// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include "checks.h"

/// This interface defines methods for reporting findings. It is to be used with the checkers, which
/// perform various checks on a CZI document.
/// Call sequence (per checker) must be: StartCheck(check) -> zero or more ReportFinding(..) -> FinishCheck(check).
/// Preconditions:
/// - Only one checker interacts with the gatherer at a time (no concurrent calls).
/// - The `finding.check` value passed to ReportFinding must match the currently active checker.
/// - Each checker calls StartCheck exactly once and FinishCheck exactly once.
/// Behavior:
/// - ReportFinding may return ReportFindingResult::Stop to request fail-fast behavior from the caller.
/// - Implementations should validate these rules and report violations (e.g., via exceptions).
class IResultGathererReport 
{
public:
    /// Values that represent severities of a finding.
    /// We distinguish between Fatal, Warning and Info.
    enum class Severity
    {
        Fatal,          ///< The finding is a fatal issue, i.e. the CZI-document is considered invalid and adverse behavior is expected.
        Warning,        ///< The finding is a warning, i.e.  a problem has been detected which may result in adverse behavior.
        Info,           ///< The finding is informational.
    };

    /// This enum is returned by 'ReportFinding(..)' to indicate whether processing should continue or stop.
    /// It allows an 'IResultGatherer' implementation the influence whether the caller should continue running
    /// checks and reporting additional findings, or stop early (e.g. when fail-fast mode is enabled).
    enum class ReportFindingResult
    {
        Continue,   ///< Continue reporting findings.
        Stop,       ///< Stop reporting findings (e.g. when fail-fast is enabled).
    };

    /// Represents a single finding emitted by a checker.
    /// - `check`: identifies the checker emitting the finding (must match the active checker).
    /// - `severity`: classification of the finding (defaults to Info).
    /// - `information`: short human-readable description of the issue.
    /// - `details`: optional extended information.
    struct Finding
    {
        /// Checker that emitted the finding; must match the currently active checker when reporting.
        explicit Finding(CZIChecks check) :check(check), severity(Severity::Info) {}

        /// Checker identifier associated with this finding.
        CZIChecks   check;

        /// Severity classification; defaults to Info.
        Severity    severity;

        /// Short human-readable description.
        std::string information;

        /// Optional extended details (may be empty).
        std::string details;
    };

    /// Begins reporting for the specified checker. Must be called before any findings
    /// for this checker and must not be invoked while another checker is active.
    virtual void StartCheck(CZIChecks check) = 0;

    /// Reports a single finding for the currently active checker.
    /// Returns whether the caller should continue reporting (Continue) or stop (Stop),
    /// enabling fail-fast behavior when needed.
    [[nodiscard]] virtual ReportFindingResult ReportFinding(const Finding& finding) = 0;

    /// Marks the end of reporting for the specified checker. Must be called exactly once
    /// after all findings for that checker have been reported.
    virtual void FinishCheck(CZIChecks check) = 0;

    virtual ~IResultGathererReport() = default;
    IResultGathererReport() = default;
    IResultGathererReport(const IResultGathererReport&) = delete;             // copy constructor
    IResultGathererReport& operator=(const IResultGathererReport&) = delete;  // copy assignment
    IResultGathererReport(IResultGathererReport&&) = delete;                  // move constructor
    IResultGathererReport& operator=(IResultGathererReport&&) = delete;       // move assignment
};

/// Interface covering the overall operation of a gatherer (lifecycle and aggregated statistics).
class IResultGathererControl
{
public:
    /// Interface covering the overall operation of a gatherer (lifecycle and aggregated statistics).
    struct CheckResult
    {
        CheckResult() : fatalMessagesCount(0), warningMessagesCount(0), infoMessagesCount(0) {}
        std::uint32_t fatalMessagesCount;
        std::uint32_t warningMessagesCount;
        std::uint32_t infoMessagesCount;

        std::uint32_t GetTotalMessagesCount() const { return this->fatalMessagesCount + this->warningMessagesCount + this->infoMessagesCount; }
    };

    /// Values that represent the "aggregated result" of the complete run.
    enum class AggregatedResult
    {
        OK,                 ///< No warnings or fatal errors, only info.
        WithWarnings,       ///< There have been one or more warnings, but not fatal error.
        ErrorsDetected,     ///< There have been one or more fatal errors.
    };

    /// Finalizes processing after all checkers have finished. Must be called exactly once
    /// after the last checker completes.
    virtual void FinalizeChecks() = 0;

    /// Returns the current aggregate counts; may be called at any time to obtain statistics
    /// for all findings reported so far. The returned counts reflect all findings observed up to
    /// the call and are not reset by this method.
    /// \return Totals for all findings reported so far
    virtual CheckResult GetAggregatedCounts() const = 0;

    /// Computes the aggregated result from the currently accumulated counts.
    /// \return Overall result classification based on all findings reported so far.
    AggregatedResult GetAggregatedResult() const;

    static AggregatedResult GetAggregatedResult(const CheckResult& check_result);

    virtual ~IResultGathererControl() = default;
    IResultGathererControl() = default;
    IResultGathererControl(const IResultGathererControl&) = delete;             // copy constructor
    IResultGathererControl& operator=(const IResultGathererControl&) = delete;  // copy assignment
    IResultGathererControl(IResultGathererControl&&) = delete;                  // move constructor
    IResultGathererControl& operator=(IResultGathererControl&&) = delete;       // move assignment
};

/// Combined interface for a result gatherer, encompassing both reporting and control aspects.
class IResultGatherer : public IResultGathererReport, public IResultGathererControl
{
public:
    IResultGatherer() = default;
    IResultGatherer(const IResultGatherer&) = delete;             // copy constructor
    IResultGatherer& operator=(const IResultGatherer&) = delete;  // copy assignment
    IResultGatherer(IResultGatherer&&) = delete;                  // move constructor
    IResultGatherer& operator=(IResultGatherer&&) = delete;       // move assignment
};
