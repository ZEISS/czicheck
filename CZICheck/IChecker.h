// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include "inc_libCZI.h"
#include "resultgatherer.h"
#include "checks.h"

/// The interface of a "checker class".
class IChecker
{
public:
    /// Executes the 'check' operation.
    virtual void RunCheck() = 0;

    virtual ~IChecker() = default;

    // non-copyable and non-moveable
    IChecker() = default;
    IChecker(const IChecker&) = delete;             // copy constructor
    IChecker& operator=(const IChecker&) = delete;  // copy assignment
    IChecker(IChecker&&) = delete;                  // move constructor
    IChecker& operator=(IChecker&&) = delete;       // move assignment
};
