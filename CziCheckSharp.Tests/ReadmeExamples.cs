// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp.Tests;

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

/// <summary>
/// Code from the README.md demonstrating how to use the CziCheckSharp library.
/// </summary>
public class ReadmeExamples
{
    public static void CheckAndPrintResult(string file)
    {
        // Create a checker with default configuration
        var configuration = new Configuration();
        using var checker = new CziChecker(configuration);

        // Validate a CZI file
        FileResult result = checker.Check(file);

        // Check validation status
        if (result.FileStatus == CheckStatus.Ok)
        {
            Console.WriteLine("Validation passed!");
        }
        else
        {
            Console.WriteLine($"Validation issues found in file {result.File}:");
            foreach (var checkResult in result.CheckResults.Where(x => x.Status != CheckStatus.Ok))
            {
                Console.WriteLine($"  {checkResult.Check}: {checkResult.Status}");
                foreach (var finding in checkResult.Findings)
                {
                    Console.WriteLine($"    {finding.Severity}: {finding.Description}");
                }
            }
        }
    }

    public static void Configuration(string file)
    {
        var configuration = new Configuration
        {
            Checks = Checks.Default, // Or Checks.All, or specific flags
            MaxFindings = 100,       // Limit findings per check (-1 for unlimited)
            LaxParsing = false,      // Enable tolerant CZI parsing
            IgnoreSizeM = false      // Ignore M dimension for pyramid subblocks
        };

        using var checker = new CziChecker(configuration);
        var result = checker.Check(file);
    }

    public static void GetVersion()
    {
        string version = CziChecker.GetCziCheckVersion();
        Console.WriteLine($"CZICheck version: {version}");
    }

    [Conditional("THIS_CODE_IS_NEVER_CALLED")]
    public static void SetNativeLibraryResolver()
    {
        NativeLibrary.SetDllImportResolver(
            typeof(CziChecker).Assembly,
            (libraryName, assembly, searchPath) => libraryName switch
            {
                // Load from custom path
                "libczicheckc" => NativeLibrary.Load("/custom/path/to/libczicheckc.so"),
                _ => IntPtr.Zero,
            });
    }
}