// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

internal static class CheckStatusParser
{
    public static CheckStatus Parse(string? status)
    {
        return Enum.TryParse<CheckStatus>(
            status,
            ignoreCase: true,
            out var result)
            ? result
            : CheckStatus.Fail;
    }
}
