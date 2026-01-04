// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgatherer.h"
#include "resultgathererbase.h"
#include "checkerfactory.h"
#include "checks.h"
#include "cmdlineoptions.h"
#include <ostream>
#include <sstream>
#include <utility>

using namespace std;

CResultGatherer::CResultGatherer(const CCmdLineOptions& options)
    : ResultGathererBase(options)
{
}

void CResultGatherer::StartCheck(CZIChecks check)
{
    const auto checker_display_name = CCheckerFactory::GetCheckerDisplayName(check);
    ostringstream ss;
    ss << "Test \"" << checker_display_name << "\" :";
    this->GetLog()->WriteStdOut(ss.str());

    this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
}

void CResultGatherer::FinishCheck(CZIChecks check)
{
    const auto& it = this->results_.find(check);

    const auto& result = it->second;

    if (this->GetMaxNumberOfMessagesToPrint() > 0)
    {
        const auto no_of_total_findings = result.GetTotalMessagesCount();
        if (no_of_total_findings > this->GetMaxNumberOfMessagesToPrint())
        {
            const auto findings_omitted = no_of_total_findings - max(this->GetMaxNumberOfMessagesToPrint(), 0);
            ostringstream ss;
            ss << "  <" << findings_omitted << " more finding" << (findings_omitted > 1 ? "s" : "") << " omitted> \n";
            this->GetLog()->WriteStdOut(ss.str());
        }
    }

    if (result.fatalMessagesCount == 0 && result.warningMessagesCount == 0)
    {
        this->GetLog()->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
        this->GetLog()->WriteStdOut(" OK\n");
        this->GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
    else if (result.fatalMessagesCount == 0)
    {
        this->GetLog()->SetColor(ConsoleColor::LIGHT_RED, ConsoleColor::DEFAULT);
        this->GetLog()->WriteStdOut(" WARN\n");
        this->GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
    else
    {
        this->GetLog()->SetColor(ConsoleColor::DARK_RED, ConsoleColor::DEFAULT);
        this->GetLog()->WriteStdOut(" FAIL\n");
        this->GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
}

CResultGatherer::ReportFindingResult CResultGatherer::ReportFinding(const Finding& finding)
{
    const auto it = this->results_.find(finding.check);
    const auto no_of_findings_so_far = it->second.GetTotalMessagesCount();
    IncrementCounter(finding.severity, it->second);

    if (this->GetMaxNumberOfMessagesToPrint() < 0 ||
        no_of_findings_so_far < this->GetMaxNumberOfMessagesToPrint())
    {
        if (no_of_findings_so_far == 0)
        {
            this->GetLog()->WriteStdOut("\n");
        }

        this->GetLog()->WriteStdOut("  ");
        this->GetLog()->WriteStdOut(finding.information);
        this->GetLog()->WriteStdOut("\n");
        if (this->GetPrintDetailsOfMessages() && !finding.details.empty())
        {
            this->GetLog()->WriteStdOut("  details: ");
            this->GetLog()->SetColor(ConsoleColor::LIGHT_YELLOW, ConsoleColor::DEFAULT);
            this->GetLog()->WriteStdOut(finding.details);
            this->GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
            this->GetLog()->WriteStdOut("\n");
        }
    }

    return this->DetermineReportFindingResult(finding);
}

void CResultGatherer::FinalizeChecks()
{
    switch (this->GetAggregatedResult())
    {
        case IResultGatherer::AggregatedResult::OK:
            this->GetLog()->WriteStdOut("\n\nResult: OK\n");
            break;
        case IResultGatherer::AggregatedResult::WithWarnings:
            this->GetLog()->WriteStdOut("\n\nResult: With Warnings\n");
            break;
        case IResultGatherer::AggregatedResult::ErrorsDetected:
            this->GetLog()->WriteStdOut("\n\nResult: Errors Detected\n");
            break;
        default:
            this->GetLog()->WriteStdOut("\n\nResult: something unexpected happened\n");
            break;
    }
}

