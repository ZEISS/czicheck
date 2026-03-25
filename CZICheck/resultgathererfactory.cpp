// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererfactory.h"

#include "resultgatherer.h"
#include "resultgathererjson.h"
#include "resultgathererxml.h"


std::unique_ptr<IResultGatherer> CreateResultGatherer(OutputEncodingFormat format, const ResultGathererOptions& options)
{
    switch (format)
    {
        case OutputEncodingFormat::TEXT:
            return std::make_unique<CResultGatherer>(options);
        case OutputEncodingFormat::JSON:
            return std::make_unique<CResultGathererJson>(options);
        case OutputEncodingFormat::XML:
            return std::make_unique<CResultGathererXml>(options);
        default:
            throw std::invalid_argument("Unknown output encoding format");
    }
}
