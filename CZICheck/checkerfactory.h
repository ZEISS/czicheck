// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>
#include "inc_libCZI.h"
#include "resultgatherer.h"
#include "checks.h"
#include "IChecker.h"

/// Additional information passed to a checked (i.e. besides the CZI-reader object).
/// Those pieces of information may be checker specific (i.e. only useful for specific
/// checkers). In general, the idea is that checkers should be able to run in isolation
/// and stateless. This may be adverse performance-wise because some calculations may
/// be have to be done repeatedly - so, intermediate results could be cached and re-used
/// in some other checker. However, at this point we refrain from supporting those
/// scenarios and aim for independent and stateless checkers.
/// So what could go in here is information which cannot be retrieved from the CZI-reader
/// object, and the first example is the "filesize" in bytes. This information is conceptually
/// not available in libCZI's stream-objects and is therefore a good fit for this 'additional
/// information' structure.
struct CheckerCreateInfo
{
    /// The size of the CZI-file in bytes. A value of 0 means "file size is unknown".
    /// For local files, this is determined via filesystem APIs. For HTTP/HTTPS streams, this is
    /// determined by probing reads at various offsets using binary search. If the size cannot be
    /// determined (stream too large or errors during probing), this will be 0.
    /// Checkers that depend on file size should handle this gracefully and skip validation or
    /// report an informational message when size is unavailable.
    std::uint64_t totalFileSize{ 0 };
};

/// Factory for creating checker instances.
class CCheckerFactory
{
public:
    /// Creates an instance of a checker. 
    /// Important: the lifetimes of the result-gatherer and the
    /// additional info is expected to be longer than the lifetime of the checker, and this is to
    /// be ensured by the caller.
    /// \param          check          The checker type identification.
    /// \param          reader         The CZI-reader object.
    /// \param          result_gatherer The result gatherer.
    /// \param          additional_info Additional information to be provided to the checker.
    /// \returns The newly created checker.
    static std::unique_ptr<IChecker> CreateChecker(
        CZIChecks check,
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);

    static const std::string& GetCheckerDisplayName(CZIChecks check_type);
    static bool TryParseShortName(const std::string& short_name, CZIChecks& check_type);

    /// Information about a checker.
    struct CheckersInfo
    {
        CZIChecks checkerType{};    ///< Type of the checker.
        std::string shortName;      ///< Short name of the checker.
        std::string displayName;    ///< The display name of the checker.
        bool isOptIn{ false };      ///< Whether this checker is an "opt-in" checker, meaning that it is disabled by default, and must be explicitly enabled.
    };

    /// Enumerate all available checkers.
    ///
    /// \param  enum_func    Functor which will be called for every checker. If returning false, the enumeration is immediately stopped.
    static void EnumerateCheckers(const std::function<bool(const CheckersInfo&)>& enum_func);
};
