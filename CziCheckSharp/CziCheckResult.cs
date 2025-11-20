// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Text.Json;

/// <summary>
/// Represents the result of a CZI check operation.
/// </summary>
public class CziCheckResult
{
    public string? ErrorOutput { get; init; }

    public string? OverallResult { get; init; }

    public IReadOnlyList<CheckerResult> CheckerResults { get; init; } = [];

    public IEnumerable<Finding> Findings => this.CheckerResults.SelectMany(cr => cr.Findings);

    public string? Version { get; init; }

    public static CziCheckResult FromJson(string jsonOutput, string? errorOutput = null)
    {
        try
        {
            var output = JsonSerializer.Deserialize<CziCheckJsonOutput>(jsonOutput);
            return output == null
                ? CreateErrorResult("Deserialized output is null.", errorOutput)
                : new CziCheckResult
                {
                    OverallResult = output.OverallResult,
                    CheckerResults = output.Tests ?? [],
                    Version = output.OutputVersion?.Version,
                    ErrorOutput = errorOutput
                };
        }
        catch (JsonException ex)
        {
            return CreateErrorResult(
                "Failed to parse JSON output.",
                $"{ex.Message}\nRaw output: {jsonOutput}");
        }
    }

    public static CziCheckResult CreateErrorResult(
        string constantMessage,
        string? variableMessage = null)
    {
        return new CziCheckResult
        {
            ErrorOutput = string.IsNullOrWhiteSpace(variableMessage)
                ? constantMessage
                : $"{constantMessage} {variableMessage}"
        };
    }
}