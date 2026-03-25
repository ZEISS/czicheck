// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "consoleio.h"
#include <memory>

/// Lightweight configuration consumed by result gatherers.
/// Decoupled from CCmdLineOptions so that both the CLI and WASM entry points
/// can construct it independently.
struct ResultGathererOptions
{
    /// Logger instance used for output.
    std::shared_ptr<ILog> log;

    /// Maximum number of findings to print per checker. Negative means no limit.
    int maxNumberOfFindingsToPrint{ 3 };

    /// Whether to include extended details with each finding.
    bool printDetailsOfMessages{ false };

    /// Values that control fail-fast behavior (mirrors CCmdLineOptions::FailFastMode).
    enum class FailFastMode
    {
        Disabled,
        FailFastForFatalErrorsPerChecker,
        FailFastForFatalErrorsOverall,
    };

    FailFastMode failFastMode{ FailFastMode::Disabled };
};
