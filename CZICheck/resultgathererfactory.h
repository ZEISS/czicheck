// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include "resultgathereroptions.h"
#include "IResultGatherer.h"

/// Output encoding format for result gatherers.
enum class OutputEncodingFormat
{
    TEXT,
    JSON,
    XML,
};

std::unique_ptr<IResultGatherer> CreateResultGatherer(OutputEncodingFormat format, const ResultGathererOptions& options);
