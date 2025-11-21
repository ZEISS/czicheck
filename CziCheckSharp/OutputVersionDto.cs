// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Text.Json.Serialization;

/// <summary>
/// Represents the output version information.
/// </summary>
internal record class OutputVersionDto
{
    [JsonPropertyName("command")]
    public string? Command { get; set; }

    [JsonPropertyName("version")]
    public string? Version { get; set; }
}