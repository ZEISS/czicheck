// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererbase.h"

ResultGathererBase::ResultGathererBase(const CCmdLineOptions& options)
    : options_(options)
{}

IResultGatherer::ReportFindingResult ResultGathererBase::DetermineReportFindingResult(const IResultGatherer::Finding& finding) const
{
    if (finding.severity == IResultGatherer::Severity::Fatal && 
        (this->options_.GetFailFastMode() == CCmdLineOptions::FailFastMode::FailFastForFatalErrorsOverall ||
        this->options_.GetFailFastMode() == CCmdLineOptions::FailFastMode::FailFastForFatalErrorsPerChecker))
    {
        return IResultGatherer::ReportFindingResult::Stop;
    }

    return IResultGatherer::ReportFindingResult::Continue;
}

void ResultGathererBase::CoreStartCheck(CZIChecks check)
{
    if (this->current_checker_.has_value())
    {
        throw std::runtime_error("A checker is already active; cannot start a new check.");
    }

    auto insert_result = this->results_.insert(std::pair(check, IResultGatherer::CheckResult()));
    if (!insert_result.second)
    {
        throw std::runtime_error("Attempting to run a check multiple times.");
    }

    this->current_checker_ = check;
}

void ResultGathererBase::CoreReportFinding(const IResultGatherer::Finding& finding)
{
    if (!this->current_checker_.has_value())
    {
        throw std::runtime_error("No currently active checker.");
    }

    if (finding.check != this->current_checker_.value())
    {
        throw std::runtime_error("The finding's check does not match the currently active checker.");
    }

    const auto it = this->results_.find(this->current_checker_.value());
    if (it == this->results_.end())
    {
        throw std::runtime_error("No results found for the currently active checker.");
    }

    switch (finding.severity)
    {
    case IResultGatherer::Severity::Fatal:
        ++(it->second.fatalMessagesCount);
        break;
    case IResultGatherer::Severity::Warning:
        ++(it->second.warningMessagesCount);
        break;
    case IResultGatherer::Severity::Info:
        ++(it->second.infoMessagesCount);
        break;
    }
}

void ResultGathererBase::CoreFinishCheck(CZIChecks check)
{
    this->current_checker_ = std::nullopt;
}

IResultGatherer::CheckResult ResultGathererBase::GetCheckResultForCurrentlyActiveChecker() const
{
    if (!this->current_checker_.has_value())
    {
        throw std::runtime_error("No currently active checker.");
    }

    const auto& it = this->results_.find(this->current_checker_.value());
    if (it == this->results_.end())
    {
        throw std::runtime_error("No results found for the currently active checker.");
    }

    return it->second;
}

/*IResultGatherer::AggregatedResult ResultGathererBase::CoreGetAggregatedResult() const
{
    std::uint32_t total_fatal_messages_count = 0;
    std::uint32_t total_warning_messages_count = 0;
    std::uint32_t total_info_messages_count = 0;
    for (auto const& i : this->results_)
    {
        total_fatal_messages_count += i.second.fatalMessagesCount;
        total_warning_messages_count += i.second.warningMessagesCount;
        total_info_messages_count += i.second.infoMessagesCount;
    }

    if (total_fatal_messages_count > 0)
    {
        return IResultGatherer::AggregatedResult::ErrorsDetected;
    }

    if (total_warning_messages_count > 0)
    {
        return IResultGatherer::AggregatedResult::WithWarnings;
    }

    return IResultGatherer::AggregatedResult::OK;
}*/

IResultGatherer::CheckResult ResultGathererBase::CoreGetAggregatedCounts() const
{
    IResultGatherer::CheckResult aggregated_result;
    for (auto const& i : this->results_)
    {
        aggregated_result.fatalMessagesCount += i.second.fatalMessagesCount;
        aggregated_result.warningMessagesCount += i.second.warningMessagesCount;
        aggregated_result.infoMessagesCount += i.second.infoMessagesCount;
    }

    return aggregated_result;
}
