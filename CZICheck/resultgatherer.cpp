// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgatherer.h"
#include "checkerfactory.h"
#include "checks.h"
#include "cmdlineoptions.h"
#include <algorithm>
#include <ostream>
#include <sstream>
#include <utility>

using namespace std;

CResultGatherer::CResultGatherer(const CCmdLineOptions& options)
    : options_(options)
{
}

void CResultGatherer::StartCheck(CZIChecks check)
{
    const auto checker_display_name = CCheckerFactory::GetCheckerDisplayName(check);
    ostringstream ss;
    ss << "Test \"" << checker_display_name << "\" :";
    this->options_.GetLog()->WriteStdOut(ss.str());

    this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
}

void CResultGatherer::FinishCheck(CZIChecks check)
{
    const auto& it = this->results_.find(check);

    const auto& result = it->second;

    if (this->options_.GetMaxNumberOfMessagesToPrint() > 0)
    {
        const auto no_of_total_findings = result.GetTotalMessagesCount();
        if (no_of_total_findings > this->options_.GetMaxNumberOfMessagesToPrint())
        {
            const auto findings_omitted = no_of_total_findings - max(this->options_.GetMaxNumberOfMessagesToPrint(), 0);
            ostringstream ss;
            ss << "  <" << findings_omitted << " more finding" << (findings_omitted > 1 ? "s" : "") << " omitted> \n";
            this->options_.GetLog()->WriteStdOut(ss.str());
        }
    }

    if (result.fatalMessagesCount == 0 && result.warningMessagesCount == 0)
    {
        this->options_.GetLog()->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
        this->options_.GetLog()->WriteStdOut(" OK\n");
        this->options_.GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
    else if (result.fatalMessagesCount == 0)
    {
        this->options_.GetLog()->SetColor(ConsoleColor::LIGHT_RED, ConsoleColor::DEFAULT);
        this->options_.GetLog()->WriteStdOut(" WARN\n");
        this->options_.GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
    else
    {
        this->options_.GetLog()->SetColor(ConsoleColor::DARK_RED, ConsoleColor::DEFAULT);
        this->options_.GetLog()->WriteStdOut(" FAIL\n");
        this->options_.GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
}

bool CResultGatherer::ReportFinding(const Finding& finding)
{
    const auto it = this->results_.find(finding.check);
    const auto no_of_findings_so_far = it->second.GetTotalMessagesCount();
    IncrementCounter(finding.severity, it->second);

    if (this->options_.GetMaxNumberOfMessagesToPrint() < 0 ||
        no_of_findings_so_far < this->options_.GetMaxNumberOfMessagesToPrint())
    {
        if (no_of_findings_so_far == 0)
        {
            this->options_.GetLog()->WriteStdOut("\n");
        }

        this->options_.GetLog()->WriteStdOut("  ");
        this->options_.GetLog()->WriteStdOut(finding.information);
        this->options_.GetLog()->WriteStdOut("\n");
        if (this->options_.GetPrintDetailsOfMessages() && !finding.details.empty())
        {
            this->options_.GetLog()->WriteStdOut("  details: ");
            this->options_.GetLog()->SetColor(ConsoleColor::LIGHT_YELLOW, ConsoleColor::DEFAULT);
            this->options_.GetLog()->WriteStdOut(finding.details);
            this->options_.GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
            this->options_.GetLog()->WriteStdOut("\n");
        }
    }

    // Check if we should fail-fast after reporting this finding
    if (this->IsFailFastEnabled() && finding.severity == IResultGatherer::Severity::Fatal)
    {
        this->NotifyFailFastStop(finding.check);
        return false; // Signal to stop
    }

    return true; // Continue
}

bool CResultGatherer::HasFatal(CZIChecks check) const
{
    const auto it = this->results_.find(check);
    if (it == this->results_.end()) return false;
    return it->second.fatalMessagesCount > 0;
}

bool CResultGatherer::IsFailFastEnabled() const
{
    return this->options_.GetFailFastEnabled();
}

void CResultGatherer::NotifyFailFastStop(CZIChecks check)
{
    // Print a single line explaining why we stopped
    this->options_.GetLog()->WriteStdOut("  Fail-fast: stopping after first error\n");
}

void CResultGatherer::FinalizeChecks()
{
    switch (this->GetAggregatedResult())
    {
        case IResultGatherer::AggregatedResult::OK:
            this->options_.GetLog()->WriteStdOut("\n\nResult: OK\n");
            break;
        case IResultGatherer::AggregatedResult::WithWarnings:
            this->options_.GetLog()->WriteStdOut("\n\nResult: With Warnings\n");
            break;
        case IResultGatherer::AggregatedResult::ErrorsDetected:
            this->options_.GetLog()->WriteStdOut("\n\nResult: Errors Detected\n");
            break;
        default:
            this->options_.GetLog()->WriteStdOut("\n\nResult: something unexpected happened\n");
            break;
    }
}

