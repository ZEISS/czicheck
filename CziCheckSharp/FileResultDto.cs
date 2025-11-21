// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;

/// <summary>
/// Represents the JSON output structure from the CZICheck
/// command-line utility.
/// </summary>
internal class FileResultDto
{
    [JsonPropertyName("aggregatedresult")]
    public string? AggregatedResult { get; set; }

    [JsonPropertyName("tests")]
    public List<CheckResultDto> Tests { get; set; } = [];

    [JsonPropertyName("output_version")]
    public OutputVersionDto? OutputVersion { get; set; }

    public FileResult ToResultFor(string file)
    {
        return new FileResult(
            file,
            CheckStatusParser.Parse(this.AggregatedResult),
            [..from t in this.Tests select t.ToCheckResult()]);
    }

    public static FileResultDto FromJson(
        string jsonOutput,
        string? additionalErrorInfo = null)
    {
        try
        {
            var output = JsonSerializer.Deserialize<FileResultDto>(jsonOutput);
            return output ?? throw new JsonException(
                "Deserialized output is null.");
        }
        catch (JsonException ex)
        {
            StringBuilder message = new(ex.Message);
            if (!string.IsNullOrWhiteSpace(additionalErrorInfo))
            {
                _ = message.AppendLine()
                    .Append("Additional info: ")
                    .Append(additionalErrorInfo);
            }

            _ = message
                .AppendLine()
                .AppendLine("Raw Json:")
                .Append(jsonOutput);

            throw new JsonException(message.ToString(), ex);
        }
    }
}
