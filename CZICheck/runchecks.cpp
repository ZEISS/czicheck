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
        stream = libCZI::CreateStreamFromFile(this->opts.GetCZIFilename().c_str());
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
    checkerAdditionalInfo.totalFileSize = GetFileSize(this->opts.GetCZIFilename().c_str());

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
