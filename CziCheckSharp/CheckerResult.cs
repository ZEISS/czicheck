// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Text.Json.Serialization;

/// <summary>
/// Represents the result of a specific checker.
/// </summary>
public class CheckerResult
{
    /// <summary>
    /// Gets or sets the name of the test (for some reason, this is not the
    /// same as the checker name).
    /// </summary>
    [JsonPropertyName("name")]
    public string? TestName { get; set; }

    [JsonPropertyName("description")]
    public string? Description { get; set; }

    [JsonPropertyName("result")]
    public string? Result { get; set; }

    [JsonPropertyName("findings")]
    public List<Finding> Findings { get; set; } = [];
}