// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include "checks.h"

#if false
/// This interface defines methods for gathering results from various checks performed on a CZI document.
/// The semantics of the methods are as follows:
/// - StartCheck(CZIChecks check): Called when a specific check is started.  
/// - ReportFinding(const Finding& finding): Called to report a finding from a check. This may be called zero or more times per check.  
///    It is important that the `finding.check` member matches the check passed to StartCheck. It is an error if it does not. The method 
///    returns a ReportFindingResult enum, which indicates whether processing (in this current checker) should continue or stop.
/// - FinishCheck(CZIChecks check): Called when a specific check is finished.  
///
/// This sequence of calls (StartCheck -> ReportFinding* -> FinishCheck) is repeated for each check performed. It is illegal to call
/// into result-gatherer from the same checker multiple times (i.e., call StartCheck with the same `check` argument multiple times).
/// Operation is strictly serial; no concurrent calls into the result gatherer must be made.
/// 
class IResultGatherer
{
public:
    struct CheckResult
    {
        CheckResult() : fatalMessagesCount(0), warningMessagesCount(0), infoMessagesCount(0) {}
        std::uint32_t fatalMessagesCount;
        std::uint32_t warningMessagesCount;
        std::uint32_t infoMessagesCount;

        std::uint32_t GetTotalMessagesCount() const { return this->fatalMessagesCount + this->warningMessagesCount + this->infoMessagesCount; }
    };


    /// Values that represent severities of a finding.
    /// We distinguish between Fatal, Warning and Info.
    enum class Severity
    {
        Fatal,          ///< The finding is a fatal issue, i.e. the CZI-document is considered invalid and adverse behavior is expected.
        Warning,        ///< The finding is a warning, i.e.  a problem has been detected which may result in adverse behavior.
        Info,           ///< The finding is informational.
    };

    /// Values that represent the "aggregated result" of the complete run.
    enum class AggregatedResult
    {
        OK,                 ///< No warnings or fatal errors, only info.
        WithWarnings,       ///< There have been one or more warnings, but not fatal error.
        ErrorsDetected,     ///< There have been one or more fatal errors.
    };

    /// This enum is returned by 'ReportFinding(..)' to indicate whether processing should continue or stop.
    /// It allows an 'IResultGatherer' implementation the influence whether the caller should continue running
    /// checks and reporting additional findings, or stop early (e.g. when fail-fast mode is enabled).
    enum class ReportFindingResult
    {
        Continue,   ///< Continue reporting findings.
        Stop,       ///< Stop reporting findings (e.g. when fail-fast is enabled).
    };

    struct Finding
    {
        explicit Finding(CZIChecks check) :check(check), severity(Severity::Info) {}
        CZIChecks   check;
        Severity    severity;
        std::string information;
        std::string details;
        constexpr const char* FindingSeverityToString() const
        {
            switch (severity)
            {
                case Severity::Info: return "INFO";
                case Severity::Warning: return "WARNING";
                case Severity::Fatal: return "FATAL";
                default: return "UNKNOWN";
            }
        }
    };

  /*  virtual void StartCheck(CZIChecks check) = 0;
    [[nodiscard]] virtual ReportFindingResult ReportFinding(const Finding& finding) = 0;
    virtual void FinishCheck(CZIChecks check) = 0;*/
    virtual void FinalizeChecks() = 0;
    //virtual AggregatedResult GetAggregatedResult() const = 0;
    virtual ~IResultGatherer() = default;
    IResultGatherer() = default;
    IResultGatherer(const IResultGatherer&) = delete;             // copy constructor
    IResultGatherer& operator=(const IResultGatherer&) = delete;  // copy assignment
    IResultGatherer(IResultGatherer&&) = delete;                  // move constructor
    IResultGatherer& operator=(IResultGatherer&&) = delete;       // move assignment
};
#endif

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
    /*
    /// Values that represent the "aggregated result" of the complete run.
    enum class AggregatedResult
    {
        OK,                 ///< No warnings or fatal errors, only info.
        WithWarnings,       ///< There have been one or more warnings, but not fatal error.
        ErrorsDetected,     ///< There have been one or more fatal errors.
    };*/

    /// This enum is returned by 'ReportFinding(..)' to indicate whether processing should continue or stop.
    /// It allows an 'IResultGatherer' implementation the influence whether the caller should continue running
    /// checks and reporting additional findings, or stop early (e.g. when fail-fast mode is enabled).
    enum class ReportFindingResult
    {
        Continue,   ///< Continue reporting findings.
        Stop,       ///< Stop reporting findings (e.g. when fail-fast is enabled).
    };

    struct Finding
    {
        explicit Finding(CZIChecks check) :check(check), severity(Severity::Info) {}
        CZIChecks   check;
        Severity    severity;
        std::string information;
        std::string details;
        constexpr const char* FindingSeverityToString() const
        {
            switch (severity)
            {
            case Severity::Info: return "INFO";
            case Severity::Warning: return "WARNING";
            case Severity::Fatal: return "FATAL";
            default: return "UNKNOWN";
            }
        }
    };

    virtual void StartCheck(CZIChecks check) = 0;
    [[nodiscard]] virtual ReportFindingResult ReportFinding(const Finding& finding) = 0;
    virtual void FinishCheck(CZIChecks check) = 0;
};

class IResultGathererControl
{
public:
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

    virtual void FinalizeChecks() = 0;
    virtual CheckResult GetAggregatedCounts() const = 0;

    AggregatedResult GetAggregatedResult() const;
    static AggregatedResult GetAggregatedResult(const CheckResult& check_result);
    //virtual AggregatedResult GetAggregatedResult() const = 0;
};

class IResultGatherer : public IResultGathererReport, public IResultGathererControl
{
};
