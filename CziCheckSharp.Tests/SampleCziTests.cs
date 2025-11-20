// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp.Tests;

using System;
using System.Runtime.CompilerServices;
using System.Text.Json;

/// <summary>
/// This test class runs the same tests as
/// $(repo)/CZICheck/test/CZICheckRunTests.py,
/// using the C# wrapper of czi-check.
/// It also uses the same CZI sample files, downloading them if necessary.
/// </summary>
public class SampleCziTests
{
    // From $(repo)/CZICheck/CMakeLists.txt
    private static readonly string[] TestDataRepos = 
    [
        "https://libczirwtestdata.z13.web.core.windows.net/CZICheckSamples/MD5/",
        "https://github.com/ptahmose/libCZI_testdata/raw/main/MD5/",
    ];

    /// <summary>
    /// Tests that we have at least one test case.
    /// </summary>
    /// <remarks>
    /// This test is useful for debugging the generation of the theory data.
    /// </remarks>
    [Fact]
    public void TheoryDataIsNotEmpty()
    {
        var testData = GetSampleCziTestData();
        _ = testData.Should().NotBeEmpty();
    }

    [Theory]
    [MemberData(nameof(GetSampleCziTestData))]
    public async Task RunAll(
        string cziFilePath,
        string md5Content,
        string expectedJsonContent)
    {
        // ARRANGE
        _ = expectedJsonContent;
        var md5 = md5Content.Trim();
        await Ensure(cziFilePath, md5);
        _ = GetFileMd5(cziFilePath).Should().Be(md5);

        CziCheckResult actual;
        var config = new Configuration
        {
            LaxParsing = true,
            Checks = Checks.All,
        };

        // ACT
        using (var sut = new CziChecker(config))
        {
            actual = sut.Check(cziFilePath);
        }

        // ASSERT
        var expected = CziCheckResult.FromJson(
            expectedJsonContent,
            actual.ErrorOutput); // We don't have an expected error output

        try
        {
            _ = actual.Should().BeEquivalentTo(expected);
        }
        catch
        {
            Console.WriteLine("==ACTUAL==");
            Console.WriteLine(JsonSerializer.Serialize(actual));
            Console.WriteLine("==EXPECTED==");
            Console.WriteLine(JsonSerializer.Serialize(expected));
            throw;
        }
    }

    public static TheoryData<string, string, string> GetSampleCziTestData()
    {
        return GetSampleCziTestDataCore();
    }

    private static TheoryData<string, string, string> GetSampleCziTestDataCore(
        [CallerFilePath] string? sourceFilePath = null)
    {
        var cziCheckSamplesPath = GetTestDataFolder(sourceFilePath);
        var testData = new TheoryData<string, string, string>();

        if (!Directory.Exists(cziCheckSamplesPath))
        {
            return testData;
        }

        // Find all .czi.md5 files
        var md5Files = Directory.GetFiles(cziCheckSamplesPath, "*.czi.md5");

        foreach (var md5File in md5Files)
        {
            // Get the base name (without .czi.md5 extension)
            var baseName = Path.GetFullPath(md5File[..^".czi.md5".Length]);
            var jsonFile = baseName + ".txt.json";

            // Only add if both files exist
            if (File.Exists(jsonFile))
            {
                var cziFile = baseName + ".czi";
                var md5Content = File.ReadAllText(md5File);
                var jsonContent = File.ReadAllText(jsonFile);
                testData.Add(cziFile, md5Content, jsonContent);
            }
        }

        return testData;
    }

    private static string GetTestDataFolder(string? sourceFilePath)
    {
        var sourceDirectory = Path.GetDirectoryName(sourceFilePath) ?? string.Empty;

        // Construct the test data folder path
        var repoRoot = Path.GetFullPath(
            Path.Combine(sourceDirectory, ".."));

        var cziCheckSamplesPath =
            Path.Combine(repoRoot, "Test", "CZICheckSamples");
        return cziCheckSamplesPath;
    }

    private static async Task Ensure(string cziFilePath, string md5)
    {
        if (File.Exists(cziFilePath))
        {
            // Verify MD5 if file exists
            var actualMd5 = GetFileMd5(cziFilePath);
            if (!string.Equals(actualMd5, md5, StringComparison.OrdinalIgnoreCase))
            {
                throw new InvalidOperationException($"MD5 mismatch for existing file: {cziFilePath}. Expected: {md5}, Actual: {actualMd5}");
            }
            return;
        }

        // Try each base URL until one succeeds
        Exception? lastException = null;
        
        foreach (var baseUrl in TestDataRepos)
        {
            try
            {
                var url = $"{baseUrl}{md5}";
                using var httpClient = new HttpClient
                {
                    Timeout = TimeSpan.FromMinutes(1)
                };
                
                using var response = await httpClient.GetAsync(url);
                _ = response.EnsureSuccessStatusCode();

                // Stream directly to file
                using (var fileStream = new FileStream(
                    cziFilePath,
                    FileMode.Create,
                    FileAccess.Write,
                    FileShare.None))
                {
                    await response.Content.CopyToAsync(fileStream);
                }

                return; // Success
            }
            catch (HttpRequestException ex)
            {
                lastException = ex;
                // Continue to next URL
            }
            catch (TaskCanceledException ex)
            {
                lastException = ex;
                // Continue to next URL (timeout or cancellation)
            }
        }
        
        // If we get here, all URLs failed
        throw new InvalidOperationException(
            $"Failed to download file from any base URL for MD5: {md5}", 
            lastException);
    }

    private static string GetFileMd5(string filePath)
    {
        using var md5 = System.Security.Cryptography.MD5.Create();
        using var stream = File.OpenRead(filePath);
        var hash = md5.ComputeHash(stream);
        return BitConverter.ToString(hash).Replace("-", string.Empty).ToLowerInvariant();
    }
}