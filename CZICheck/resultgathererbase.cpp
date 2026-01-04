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
