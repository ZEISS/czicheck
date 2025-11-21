// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

internal partial class FindingSeverityParser
{
    public static FindingSeverity Parse(string? severity)
    {
        return (severity?.ToLowerInvariant()) switch
        {
            "info" => FindingSeverity.Info,
            "warning" => FindingSeverity.Warning,
            _ => FindingSeverity.Error,
        };
    }
}
