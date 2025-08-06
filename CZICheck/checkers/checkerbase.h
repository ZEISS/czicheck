// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "inc_libCZI.h"
#include "../checkerfactory.h"
#include <memory>
#include <utility>

/// Base class for implementing a checker - this class stores the constructor-arguments
/// as properties.
class CCheckerBase
{
protected:
    std::shared_ptr<libCZI::ICZIReader> reader_;
    IResultGatherer& result_gatherer_;
    const CheckerCreateInfo& additional_info_;
public:
    CCheckerBase(
        std::shared_ptr<libCZI::ICZIReader> reader,
        IResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info) :
        reader_(std::move(reader)),
        result_gatherer_(result_gatherer),
        additional_info_(additional_info)
    {}

protected:
    /// Try to get libCZI-metadata-object. If there is any problem with this
    /// (e.g. invalid XML or so), then the error is reported (to this instance's
    /// resultGatherer-object) and a nullptr is returned. This method will not
    /// throw an exception in case of any malfunction.
    ///
    /// \param  check   The checker-identifier (used for reporting errors).
    ///
    /// \returns    If successful, the libCZI-metadata-object; nullptr otherwise.
    std::shared_ptr<libCZI::ICziMetadata> GetCziMetadataAndReportErrors(CZIChecks check);
};
