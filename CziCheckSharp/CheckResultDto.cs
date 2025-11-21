// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System;
using System.Globalization;
using System.Text;
using System.Text.Json.Serialization;

/// <summary>
/// Represents the result of a specific check.
/// </summary>
internal class CheckResultDto
{
    /// <summary>
    /// Gets or sets the name of the test (for some reason, this is not the
    /// same as the checker name).
    /// </summary>
    [JsonPropertyName("name")]
    public string? Name { get; set; }

    [JsonPropertyName("description")]
    public string? Description { get; set; }

    [JsonPropertyName("result")]
    public string? Result { get; set; }

    [JsonPropertyName("findings")]
    public List<FindingDto> Findings { get; set; } = [];

    public CheckResult ToCheckResult()
    {
        if (!ChecksParser.TryParse(this.Name, out var check))
        {
            check = Checks.None;
        }

        return new CheckResult(
            check,
            NormalizeDescription(this.Description ?? string.Empty),
            CheckStatusParser.Parse(this.Result),
            [..from f in this.Findings select f.ToFinding()]);
    }

    private static string NormalizeDescription(string v)
    {
        if (string.IsNullOrWhiteSpace(v))
        {
            return string.Empty;
        }

        var result = v.Trim();

        bool isFirstCharLower =
            char.IsLower(result, 0);
        bool mustAppendPeriod = result.Any(x => x == '.') &&
            result[^1] != '.';

        if (isFirstCharLower || mustAppendPeriod)
        {
            var normalized = new StringBuilder(result);
            var en = CultureInfo.GetCultureInfo("en-US");
            if (isFirstCharLower)
            {
                normalized[0] = char.ToUpper(normalized[0], en);
            }

            if (mustAppendPeriod)
            {
                _ = normalized.Append('.');
            }

            result = normalized.ToString();
        }

        return result;
    }
}
