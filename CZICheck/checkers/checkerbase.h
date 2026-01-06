// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <inc_libCZI.h>
#include "../checkerfactory.h"
#include "checkerexception.h"
#include <memory>
#include <utility>

/// Base class for implementing a checker - this class stores the constructor-arguments
/// as properties.
class CCheckerBase
{
protected:
    std::shared_ptr<libCZI::ICZIReader> reader_;
    IResultGathererReport& result_gatherer_;
    const CheckerCreateInfo& additional_info_;
public:
    CCheckerBase(
        std::shared_ptr<libCZI::ICZIReader> reader,
        IResultGathererReport& result_gatherer,
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

    /// Throws a CheckerException if the finding result indicates to stop processing.
    /// This method checks if the result_gatherer has requested to stop further processing
    /// and throws an exception accordingly. This is typically used after reporting findings
    /// to respect user-requested limits on reported issues.
    ///
    /// \param  result  The result returned from reporting a finding to the result gatherer.
    ///
    /// \throws CheckerException with reason StopFurtherProcessing if result is Stop.
    void ThrowIfFindingResultIsStop(IResultGatherer::ReportFindingResult result) const;

    /// Executes a callable and handles CheckerException.
    /// This template accepts any callable (lambda, function pointer, functor)
    /// and provides default exception handling for CheckerException.
    /// \tparam Callable Type of the callable object.
    /// \param func The callable to execute.
    template <typename Callable>
    void RunCheckDefaultExceptionHandling(Callable&& func)
    {
        try
        {
            func();
        }
        catch (CheckerException& checker_exception)
        {
            // If the reason is not "StopFurtherProcessing", re-throw the exception,
            // otherwise just swallow it. This "StopFurtherProcessing" is used to
            // indicate that the checker was instructed to stop processing further findings -
            // this is not an error condition per se.
            if (checker_exception.GetReason() != CheckerException::Reason::StopFurtherProcessing)
            {
                throw;
            }
        }
    }
};
