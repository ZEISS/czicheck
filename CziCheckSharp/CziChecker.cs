// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Buffers;
using System.Diagnostics;
using System.Text;
using System.Text.Json;

using static CziCheckSharp.CziCheckResult;

/// <summary>
/// Validates CZI files.
/// </summary>
/// <param name="configuration">Configuration options for CZI validation.</param>
/// <threadsafety static="true" instance="false"/>
public class CziChecker(Configuration configuration) : IDisposable
{
    private readonly Validator validator = new(configuration
        ?? throw new ArgumentNullException(nameof(configuration)));

    private byte[] outputStringBuffer = new byte[48000];
    private readonly byte[] errorMessageBuffer = new byte[2048];

    public bool IsDisposed => this.validator.IsDisposed;

    public Configuration Configuration { get; } = configuration
        ?? throw new ArgumentNullException(nameof(configuration));

    /// <summary>
    /// Gets the version information of the CZICheck library.
    /// </summary>
    /// <returns>Version string from CZICheck.</returns>
    public unsafe static string GetVersion()
    {
        int size = 1024;
        var pool = ArrayPool<byte>.Shared;
        for (int i = 0; i < 2; i++)
        {
            // Try to get the version string
            byte[] buffer = pool.Rent(size);
            size = buffer.Length;
            try
            {
                fixed (byte* pBuffer = buffer)
                {
                    ulong bufferSize = (ulong)size;
                    ulong sizeRef = bufferSize;

                    if (NativeMethods.GetLibVersionString((nint)pBuffer, ref sizeRef))
                    {
                        return GetUtf8StringFromBuffer(buffer);
                    }

                    // Buffer too small, resize and retry
                    if (sizeRef > bufferSize && sizeRef <= int.MaxValue)
                    {
                        size = (int)sizeRef;
                        continue;
                    }
                }
            }
            finally
            {
                pool.Return(buffer);
            }
        }

        // Fallback to numeric version
        NativeMethods.GetLibVersion(out int major, out int minor, out int patch);
        return $"{major}.{minor}.{patch}";
    } 

    /// <summary>
    /// Checks a CZI file with the specified options.
    /// </summary>
    /// <param name="cziFilePath">Path to the CZI file to check.</param>
    /// <returns>Result of the check operation.</returns>
    /// <exception cref="ArgumentException">
    /// Thrown when <paramref name="cziFilePath"/> is null or empty.
    /// </exception>
    /// <exception cref="FileNotFoundException">
    /// Thrown when the specified CZI file does not exist.
    /// </exception>
    public unsafe CziCheckResult Check(string cziFilePath)
    {
        ObjectDisposedException.ThrowIf(this.validator.IsDisposed, this);

        if (string.IsNullOrWhiteSpace(cziFilePath))
        {
            throw new ArgumentException(
                "CZI file path must be specified.",
                nameof(cziFilePath));
        }

        if (!File.Exists(cziFilePath))
        {
            throw new FileNotFoundException(cziFilePath);
        }

        // Try validation up to 2 times (once with initial buffer, once after resize if needed)
        for (int i = 0; i < 2; i++)
        {
            ulong outputBufferSize = (ulong)this.outputStringBuffer.Length;
            ulong errorMessageLength = (ulong)this.errorMessageBuffer.Length;

            fixed (byte* pOutputBuffer = this.outputStringBuffer)
            fixed (byte* pErrorBuffer = this.errorMessageBuffer)
            {
                int result = this.validator.ValidateFile(
                    cziFilePath,
                    (nint)pOutputBuffer,
                    ref outputBufferSize,
                    (nint)pErrorBuffer,
                    ref errorMessageLength);

                Trace.Assert(
                    result != 3,
                    "Invalid validator pointer? This is a bug.");

                // If buffer is too small and this is first attempt, grow and retry
                if (result == 1 && i == 0)
                {
                    this.outputStringBuffer = new byte[(int)outputBufferSize];
                    continue;
                }

                // Process results
                string? jsonOutput = null;
                string? errorOutput = null;

                if (outputBufferSize > 0 && result == 0)
                {
                    jsonOutput = GetUtf8StringFromBuffer(this.outputStringBuffer);
                }

                if (errorMessageLength > 0)
                {
                    errorOutput = GetUtf8StringFromBuffer(this.errorMessageBuffer);
                }

                // Handle different result codes
                return result switch
                {
                    0 => ParseJsonOutput(jsonOutput, errorOutput),
                    2 => CreateErrorResult(
                        "File access error: Could not open or read the CZI file.",
                        errorOutput),
                    4 => CreateErrorResult(
                        "One or more requested checks are not available.",
                        errorOutput),
                    _ => CreateErrorResult(
                        $"Validation failed with error code {result}.",
                        errorOutput)
                };
            }
        }

        // This should never be reached, but just in case
        throw new InvalidOperationException("Failed to validate file after retries.");
    }

    private static string GetUtf8StringFromBuffer(byte[] buffer)
    {
        // Find the null terminator
        int length = Array.IndexOf(buffer, (byte)0);
        if (length == -1)
        {
            length = buffer.Length;
        }

        return Encoding.UTF8.GetString(buffer, 0, length);
    }

    private static CziCheckResult ParseJsonOutput(
        string? jsonOutput,
        string? errorOutput)
    {
        if (string.IsNullOrWhiteSpace(jsonOutput))
        {
            return CreateErrorResult(
                "No output received from validation.",
                errorOutput);
        }

        return CziCheckResult.FromJson(jsonOutput, errorOutput);
    }

    /// <summary>
    /// Disposes the CziChecker and releases native resources.
    /// </summary>
    public void Dispose()
    {
        this.validator.Dispose();
        GC.SuppressFinalize(this);
    }
}