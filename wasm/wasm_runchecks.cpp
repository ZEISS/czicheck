// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "wasm_runchecks.h"

#include "checkerfactory.h"
#include "resultgathererfactory.h"
#include "wasm_log.h"
#include <emscripten.h>
#include <libCZI.h>
#include <sstream>

namespace wasm {

bool RunChecks(
    const std::shared_ptr<libCZI::IStream>& stream,
    const RunChecksConfig& config,
    IResultGatherer::AggregatedResult& aggregated)
{
    const auto reader = libCZI::CreateCZIReader();

    try
    {
        libCZI::ICZIReader::OpenOptions open_options;
        open_options.lax_subblock_coordinate_checks = config.laxParsing;
        open_options.ignore_sizem_for_pyramid_subblocks = config.ignoreSizeMForPyramidSubblocks;
        reader->Open(stream, &open_options);
    }
    catch (std::exception& ex)
    {
        if (config.gathererOptions.log)
        {
            std::ostringstream ss;
            ss << "Could not open the CZI: " << ex.what();
            config.gathererOptions.log->WriteLineStdErr(ss.str());
        }
        return false;
    }

    CheckerCreateInfo checkerAdditionalInfo;
    checkerAdditionalInfo.totalFileSize = config.totalFileSize;

    std::vector<CZIChecks> checksToRun = config.checksToRun;
    if (checksToRun.empty())
    {
        CCheckerFactory::EnumerateCheckers(
            [&](const CCheckerFactory::CheckersInfo& info) -> bool
            {
                if (!info.isOptIn)
                {
                    checksToRun.push_back(info.checkerType);
                }
                return true;
            });
    }

    // We run each checker with its own gatherer so we can emit per-check results
    // to JS immediately, while accumulating totals for the aggregated result.
    IResultGatherer::CheckResult totalCounts{};
    bool stopped = false;

    for (auto checkType : checksToRun)
    {
        // Each check gets a fresh log + gatherer so we can serialize its result independently.
        auto perCheckLog = std::make_shared<wasm::StringLog>();
        ResultGathererOptions perCheckOpts = config.gathererOptions;
        perCheckOpts.log = perCheckLog;

        auto perCheckGatherer = CreateResultGatherer(config.outputFormat, perCheckOpts);

        // Tell JS which checker is about to run so the UI can report it on crash.
        auto checkerDisplayName = CCheckerFactory::GetCheckerDisplayName(checkType);
        EM_ASM({
            window._currentCheckerName = UTF8ToString($0);
        }, checkerDisplayName.c_str());

        auto checker = CCheckerFactory::CreateChecker(checkType, reader, *perCheckGatherer, checkerAdditionalInfo);
        if (checker)
        {
            checker->RunCheck();
        }

        // Get this check's counts and accumulate into totals.
        auto checkCounts = perCheckGatherer->GetAggregatedCounts();
        totalCounts.fatalMessagesCount += checkCounts.fatalMessagesCount;
        totalCounts.warningMessagesCount += checkCounts.warningMessagesCount;
        totalCounts.infoMessagesCount += checkCounts.infoMessagesCount;

        // Serialize this single check's result and push to JS via callback.
        perCheckGatherer->FinalizeChecks();
        std::string checkJson = perCheckLog->GetStdOut();

        if (config.onCheckComplete && !checkJson.empty())
        {
            config.onCheckComplete(checkJson);
        }

        // Yield to the browser event loop so the UI can paint the new card.
        emscripten_sleep(0);

        // Check fail-fast at the overall level.
        if (config.gathererOptions.failFastMode == ResultGathererOptions::FailFastMode::FailFastForFatalErrorsOverall &&
            totalCounts.fatalMessagesCount > 0)
        {
            stopped = true;
            break;
        }
    }

    aggregated = IResultGatherer::GetAggregatedResult(totalCounts);
    return true;
}

} // namespace wasm
