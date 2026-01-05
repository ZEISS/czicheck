// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "runchecks.h"
#include "inc_libCZI.h"
#include "utils.h"
#include "checkerfactory.h"
#include "resultgathererfactory.h"
#include <sstream>
#include <memory>
#include <utility>

using namespace std;
using namespace libCZI;

CRunChecks::CRunChecks(const CCmdLineOptions& opts, std::shared_ptr<ILog> consoleIo)
    : opts(opts), consoleIo(std::move(consoleIo))
{
}

bool CRunChecks::Run(IResultGatherer::AggregatedResult& result)
{
    shared_ptr<libCZI::IStream> stream;
    try
    {
        stream = CreateSourceStream(this->opts);
    }
    catch (exception& ex)
    {
        stringstream ss;
        ss << "Could not access the input file : " << ex.what();
        this->consoleIo->WriteLineStdErr(ss.str());
        return false;
    }

    const auto spReader = libCZI::CreateCZIReader();

    try
    {
        ICZIReader::OpenOptions options;
        options.lax_subblock_coordinate_checks = this->opts.GetLaxParsingEnabled();
        options.ignore_sizem_for_pyramid_subblocks = this->opts.GetIgnoreSizeMForPyramidSubBlocks();
        spReader->Open(stream, &options);
    }
    catch (exception& ex)
    {
        stringstream ss;
        ss << "Could not open the CZI : " << ex.what();
        this->consoleIo->WriteLineStdErr(ss.str());
        return false;
    }

    auto resultsGatherer = CreateResultGatherer(opts);

    CheckerCreateInfo checkerAdditionalInfo;
    // Determine the file size - straightforward for local files, requires probing for streams
    if (this->opts.GetSourceStreamClass().empty())
    {
        // Local file - use filesystem API
        checkerAdditionalInfo.totalFileSize = GetFileSize(this->opts.GetCZIFilename().c_str());
    }
    else
    {
        // Non-file stream (e.g., HTTP/HTTPS) - attempt to determine size by probing
        // Note: This uses binary search with reads, which may be expensive for network streams
        // If performance is a concern, this can be made optional via command-line flag
        checkerAdditionalInfo.totalFileSize = TryGetStreamSize(stream.get());
    }

    const auto& checksToRun = this->opts.GetChecksEnabled();
    for (auto checkType : checksToRun)
    {
        auto checker = CCheckerFactory::CreateChecker(checkType, spReader, *resultsGatherer, checkerAdditionalInfo);
        checker->RunCheck();
    }

    result = resultsGatherer->GetAggregatedResult();
    resultsGatherer->FinalizeChecks();
    return true;
}
