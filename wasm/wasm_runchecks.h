// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>
#include "IResultGatherer.h"
#include "resultgathereroptions.h"
#include "resultgathererfactory.h"
#include "checks.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace wasm {

/// Callback invoked after each individual checker finishes.
/// Parameters: checker short name, result ("OK"/"WARN"/"FAIL"), JSON for this single test object.
using PerCheckCallback = std::function<void(const std::string& checkResultJson)>;

/// Configuration for running CZI checks from the WASM entry point.
struct RunChecksConfig
{
    /// The checks to run. If empty, all default checkers are used.
    std::vector<CZIChecks> checksToRun;

    /// Whether lax subblock coordinate parsing is enabled.
    bool laxParsing{ false };

    /// Whether to ignore SizeM for pyramid subblocks.
    bool ignoreSizeMForPyramidSubblocks{ false };

    /// The total file size (from JS). 0 means unknown.
    std::uint64_t totalFileSize{ 0 };

    /// Result gatherer options (log, max findings, fail-fast, etc.).
    ResultGathererOptions gathererOptions;

    /// Output encoding format.
    OutputEncodingFormat outputFormat{ OutputEncodingFormat::JSON };

    /// Optional callback invoked after each check finishes with that check's JSON result.
    PerCheckCallback onCheckComplete;
};

/// Run CZI checks using a pre-opened libCZI stream.
///
/// \param stream  A libCZI IStream backed by the WASM blob reader.
/// \param config  Configuration controlling which checks to run and how to report.
/// \param[out] aggregated  Receives the overall result classification.
///
/// \returns true if the CZI was opened and checks were executed successfully.
bool RunChecks(
    const std::shared_ptr<libCZI::IStream>& stream,
    const RunChecksConfig& config,
    IResultGatherer::AggregatedResult& aggregated);

} // namespace wasm
