// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Text.Json.Serialization;

/// <summary>
/// Represents a finding from a CZI check.
/// </summary>
internal record class FindingDto
{
    [JsonPropertyName("severity")]
    public string? Severity { get; set; }

    [JsonPropertyName("description")]
    public string? Description { get; set; }

    [JsonPropertyName("details")]
    public string? Details { get; set; }

    public Finding ToFinding()
    {
        var severity = FindingSeverityParser.Parse(this.Severity);
        string description = this.Description ?? string.Empty;

        return new Finding(
            severity,
            description,
            this.Details ?? string.Empty);
    }
}
