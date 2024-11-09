// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererfactory.h"

#include "resultgatherer.h"
#include "resultgathererjson.h"
#include "resultgathererxml.h"


std::unique_ptr<IResultGatherer> CreateResultGatherer(const CCmdLineOptions& options)
{
    switch (options.GetEncodingType())
    {
        case CCmdLineOptions::EncodingType::TEXT:
            return std::make_unique<CResultGatherer>(options);
        case CCmdLineOptions::EncodingType::JSON:
            return std::make_unique<CResultGathererJson>(options);
        case CCmdLineOptions::EncodingType::XML:
            return std::make_unique<CResultGathererXml>(options);
        default:
            throw std::invalid_argument("Unknown EncodingType");
    }
}

