// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Collections.Immutable;
using System.Numerics;

/// <summary>
/// The result of a specific check.
/// </summary>
public sealed record class CheckResult(
    Checks Check,
    string CheckDescription,
    CheckStatus Status,
    ImmutableArray<Finding> Findings)
{
    /// <summary>
    /// The single check whose result is represented by this instance
    /// or <see cref="Checks.None"/> if the information is not available.
    /// </summary>
    public Checks Check { get; } = Validate(Check);

    public bool Equals(CheckResult? other)
    {
        if (other is null)
        {
            return false;
        }

        return this.Check == other.Check
            && this.CheckDescription == other.CheckDescription
            && this.Status == other.Status
            && this.Findings.SequenceEqual(other.Findings);
    }

    public override int GetHashCode()
    {
        int result = HashCode.Combine(
            this.Check,
            this.CheckDescription,
            this.Status);
        foreach (var finding in this.Findings)
        {
            result = HashCode.Combine(result, finding);
        }

        return result;
    }

    private static Checks Validate(Checks check)
    {
        return BitOperations.PopCount((uint)check) <= 1
            ? check
            : throw new ArgumentException(
                $"{nameof(check)} must be {nameof(Checks.None)} or a single check, but {check} is more than one check.",
                nameof(check));
    }
}