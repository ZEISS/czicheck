// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "wasm_runchecks.h"

#include "checkerfactory.h"
#include "resultgathererfactory.h"
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

    auto resultsGatherer = CreateResultGatherer(config.outputFormat, config.gathererOptions);

    CheckerCreateInfo checkerAdditionalInfo;
    checkerAdditionalInfo.totalFileSize = config.totalFileSize;

    std::vector<CZIChecks> checksToRun = config.checksToRun;
    if (checksToRun.empty())
    {
        // Default: all non-opt-in checkers.
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

    for (auto checkType : checksToRun)
    {
        auto checker = CCheckerFactory::CreateChecker(checkType, reader, *resultsGatherer, checkerAdditionalInfo);
        if (checker)
        {
            checker->RunCheck();
        }

        if (config.gathererOptions.failFastMode == ResultGathererOptions::FailFastMode::FailFastForFatalErrorsOverall &&
            resultsGatherer->GetAggregatedResult() == IResultGatherer::AggregatedResult::ErrorsDetected)
        {
            break;
        }
    }

    aggregated = resultsGatherer->GetAggregatedResult();
    resultsGatherer->FinalizeChecks();
    return true;
}

} // namespace wasm
