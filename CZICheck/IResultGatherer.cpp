// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "IResultGatherer.h"

/*static*/IResultGathererControl::AggregatedResult IResultGathererControl::GetAggregatedResult(const CheckResult& check_result)
{
    if (check_result.fatalMessagesCount > 0)
    {
        return IResultGathererControl::AggregatedResult::ErrorsDetected;
    }

    if (check_result.warningMessagesCount > 0)
    {
        return IResultGathererControl::AggregatedResult::WithWarnings;
    }

    return IResultGathererControl::AggregatedResult::OK;
}

IResultGathererControl::AggregatedResult IResultGathererControl::GetAggregatedResult() const
{
    return IResultGathererControl::GetAggregatedResult(this->GetAggregatedCounts());
}
