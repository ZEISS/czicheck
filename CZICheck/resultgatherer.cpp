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

using namespace std;

CResultGatherer::CResultGatherer(const CCmdLineOptions& options)
    : ResultGathererBase(options)
{
}

void CResultGatherer::StartCheck(CZIChecks check)
{
    this->CoreStartCheck(check);

    const auto checker_display_name = CCheckerFactory::GetCheckerDisplayName(check);
    ostringstream ss;
    ss << "Test \"" << checker_display_name << "\" :";
    this->GetLog()->WriteStdOut(ss.str());
}

void CResultGatherer::FinishCheck(CZIChecks check)
{
    const CheckResult current_checker_result = this->GetCheckResultForCurrentlyActiveChecker();

    this->CoreFinishCheck(check);

    if (this->GetMaxNumberOfMessagesToPrint() > 0)
    {
        const auto no_of_total_findings = current_checker_result.GetTotalMessagesCount();
        if (no_of_total_findings > this->GetMaxNumberOfMessagesToPrint())
        {
            const auto findings_omitted = no_of_total_findings - max(this->GetMaxNumberOfMessagesToPrint(), 0);
            ostringstream ss;
            ss << "  <" << findings_omitted << " more finding" << (findings_omitted > 1 ? "s" : "") << " omitted> \n";
            this->GetLog()->WriteStdOut(ss.str());
        }
    }

    if (current_checker_result.fatalMessagesCount == 0 && current_checker_result.warningMessagesCount == 0)
    {
        this->GetLog()->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
        this->GetLog()->WriteStdOut(" OK\n");
        this->GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
    else if (current_checker_result.fatalMessagesCount == 0)
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
    const uint32_t no_of_findings_so_far = this->GetCheckResultForCurrentlyActiveChecker().GetTotalMessagesCount();

    this->CoreReportFinding(finding);

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

IResultGatherer::CheckResult CResultGatherer::GetAggregatedCounts() const
{
    return this->CoreGetAggregatedCounts();
}
