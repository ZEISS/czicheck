// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include "checks.h"

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
    enum class Severity
    {
        Fatal,          ///< The finding is a fatal issue, i. e. the CZI-document is considered invalid and adverse behavior is expected.
        Warning,        ///< The finding is a warning, i. e. a problem has been detected which may result in adverse behavior.
        Info            ///< The finding is informational.
    };

    /// Values that represent the "aggregated result" of the complete run.
    enum class AggregatedResult
    {
        OK,                 ///< No warnings or fatal errors, only info.
        WithWarnings,       ///< There have been one or more warnings, but not fatal error.
        ErrorsDetected      ///< There have been one or more fatal errors.
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

protected:
    std::map<CZIChecks, CheckResult> results_;

public:
    virtual void StartCheck(CZIChecks check) = 0;
    virtual void ReportFinding(const Finding& finding) = 0;
    virtual void FinishCheck(CZIChecks check) = 0;
    virtual void FinalizeChecks() = 0;
    virtual ~IResultGatherer() = default;
    IResultGatherer() = default;
    IResultGatherer(const IResultGatherer&) = delete;             // copy constructor
    IResultGatherer& operator=(const IResultGatherer&) = delete;  // copy assignment
    IResultGatherer(IResultGatherer&&) = delete;                  // move constructor
    IResultGatherer& operator=(IResultGatherer&&) = delete;       // move assignment

public:
    AggregatedResult GetAggregatedResult()
    {
        std::uint32_t total_fatal_messages_count = 0;
        std::uint32_t total_warning_messages_count = 0;
        std::uint32_t total_info_messages_count = 0;
        for (auto const& i : this->results_)
        {
            total_fatal_messages_count += i.second.fatalMessagesCount;
            total_warning_messages_count += i.second.warningMessagesCount;
            total_info_messages_count += i.second.warningMessagesCount;
        }

        if (total_fatal_messages_count > 0)
        {
            return AggregatedResult::ErrorsDetected;
        }

        if (total_warning_messages_count > 0)
        {
            return AggregatedResult::WithWarnings;
        }

        return AggregatedResult::OK;
    }

    static void IncrementCounter(Severity severity, CheckResult& result)
    {
        switch (severity)
        {
            case Severity::Fatal:
                ++result.fatalMessagesCount;
                break;
            case Severity::Warning:
                ++result.warningMessagesCount;
                break;
            case Severity::Info:
                ++result.infoMessagesCount;
                break;
        }
    }
};

