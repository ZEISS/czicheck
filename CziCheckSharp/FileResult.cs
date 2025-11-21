// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Collections.Immutable;
using System.Text.Json;

/// <summary>
/// Represents the result of a CZI check operation.
/// </summary>
public sealed record class FileResult(
    string File,
    CheckStatus FileStatus,
    ImmutableArray<CheckResult> CheckResults)
{
    public IEnumerable<CheckResult> this[CheckStatus status]
        => this.CheckResults.Where(cr => cr.Status == status);

    public static FileResult FromJson(string file, string json)
    {
        return FileResultDto.FromJson(json).ToResultFor(file);
    }

    public bool Equals(FileResult? other)
    {
        if (other is null)
        {
            return false;
        }

        return this.File == other.File
            && this.FileStatus == other.FileStatus
            && this.CheckResults.SequenceEqual(other.CheckResults);
    }

    public override int GetHashCode()
    {
        int result = HashCode.Combine(
            this.File,
            this.FileStatus);
        foreach (var checkResult in this.CheckResults)
        {
            result = HashCode.Combine(result, checkResult);
        }

        return result;
    }
}