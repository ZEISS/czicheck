// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdexcept>
#include <string>

/// Exception thrown (internally) by checker classes when reporting a finding
/// to the result-gatherer AND instructed by it to stop further processing
/// immediately (e.g. due to fail-fast behavior).
class CheckerException : public std::runtime_error
{
public:
    /// Reasons for throwing a CheckerException.
    enum class Reason
    {
        Unknown,                    ///< Unknown reason.
        StopFurtherProcessing,      ///< The checker was instructed to stop processing further findings.
    };

private:
    Reason reason_;

public:
    /// Constructs a CheckerException with the given reason and error message.
    /// \param reason The reason for throwing this exception.
    /// \param message Description of the error condition.
    explicit CheckerException(Reason reason, const std::string& message)
        : std::runtime_error(message), reason_(reason)
    {
    }

    /// Constructs a CheckerException with the given reason and error message.
    /// \param reason The reason for throwing this exception.
    /// \param message Description of the error condition.
    explicit CheckerException(Reason reason, const char* message)
        : std::runtime_error(message), reason_(reason)
    {
    }

    /// Gets the reason for throwing this exception.
    /// \returns The reason why this exception was thrown.
    Reason GetReason() const noexcept
    {
        return this->reason_;
    }

    /// Virtual destructor for proper cleanup in derived classes.
    ~CheckerException() noexcept override = default;

    // Copy and move operations use compiler-generated defaults
    CheckerException(const CheckerException&) = default;
    CheckerException& operator=(const CheckerException&) = default;
    CheckerException(CheckerException&&) noexcept = default;
    CheckerException& operator=(CheckerException&&) noexcept = default;
};
