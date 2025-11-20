// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Text.Json.Serialization;

/// <summary>
/// Represents the JSON output structure from the CZICheck command-line utility.
/// </summary>
internal class CziCheckJsonOutput
{
    [JsonPropertyName("aggregatedresult")]
    public string? OverallResult { get; set; }

    [JsonPropertyName("tests")]
    public List<CheckerResult> Tests { get; set; } = [];

    [JsonPropertyName("output_version")]
    public OutputVersion? OutputVersion { get; set; }
}