// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include "cmdlineoptions.h"
#include "IResultGatherer.h"

std::unique_ptr<IResultGatherer> CreateResultGatherer(const CCmdLineOptions& options);
