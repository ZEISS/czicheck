// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "cmdlineoptions.h"
#include "checks.h"
#include "IResultGatherer.h"
#include "resultgathererbase.h"

/// This class is intended to receive the findings from the individual checks. It is 
/// responsible for outputting them, and aggregating an overall result.
/// It relies on the semantic of:
/// - when a new checker starts executing, it calls into 'StartCheck'  
/// - when there is a finding to be reported, the checker calls into 'ReportFinding' (as  
///    many times as necessary)
/// - when a checker is done, it calls into 'FinishCheck'.  
/// Deviating from this semantic results in undefined behavior.
class CResultGatherer : public IResultGatherer, ResultGathererBase
{
public:
    explicit CResultGatherer(const CCmdLineOptions& options);
    void StartCheck(CZIChecks check) override;
    ReportFindingResult ReportFinding(const Finding& finding) override;
    void FinishCheck(CZIChecks check) override;
    void FinalizeChecks() override;
};
