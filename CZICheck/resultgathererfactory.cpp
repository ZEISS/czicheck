// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererfactory.h"

#include "resultgatherer.h"
#include "resultgathererjson.h"
#include "resultgathererxml.h"


std::unique_ptr<IResultGatherer> CreateResultGatherer(const CCmdLineOptions& options)
{
    switch (options.GetOutputEncodingFormat())
    {
        case CCmdLineOptions::OutputEncodingFormat::TEXT:
            return std::make_unique<CResultGatherer>(options);
        case CCmdLineOptions::OutputEncodingFormat::JSON:
            return std::make_unique<CResultGathererJson>(options);
        case CCmdLineOptions::OutputEncodingFormat::XML:
            return std::make_unique<CResultGathererXml>(options);
        default:
            throw std::invalid_argument("Unknown output encoding format");
    }
}
