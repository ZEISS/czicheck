// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Buffers;
using System.Text;

using Result = CziCheckSharp.Validator.Result;

/// <summary>
/// Validates CZI files.
/// </summary>
/// <param name="configuration">Configuration options for CZI validation.</param>
/// <threadsafety static="true" instance="false"/>
public sealed class CziChecker(Configuration configuration) : IDisposable
{
    private readonly Validator validator = new(configuration
        ?? throw new ArgumentNullException(nameof(configuration)));

    private byte[] outputStringBuffer = new byte[48000];
    private readonly byte[] errorMessageBuffer = new byte[2048];

    public bool IsDisposed => this.validator.IsInvalid;

    public Configuration Configuration { get; } = configuration
        ?? throw new ArgumentNullException(nameof(configuration));

    /// <summary>
    /// Gets the version information of the CZICheck library.
    /// </summary>
    /// <remarks>
    /// This is not the version of <see cref="CziCheckSharp"/>,
    /// but the version of the underlying CZICheck library.
    /// </remarks>
    /// <returns>Version string from CZICheck.</returns>
    public unsafe static string GetCziCheckVersion()
    {
        int size = 256;
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
    public unsafe FileResult Check(string cziFilePath)
    {
        ObjectDisposedException.ThrowIf(this.validator.IsInvalid, this);

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

        Result validatorReturnValue = default;
        ulong outputBufferSize = 0;
        ulong errorMessageLength = 0;

        // Try validation up to 2 times
        // (once with initial buffer, once after resize if needed)
        for (int attempt = 1; attempt <= 2; attempt++)
        {
            Array.Clear(this.outputStringBuffer);
            Array.Clear(this.errorMessageBuffer);
            fixed (byte* pOutputBuffer = this.outputStringBuffer)
            fixed (byte* pErrorBuffer = this.errorMessageBuffer)
            {
                outputBufferSize = (ulong)this.outputStringBuffer.Length;
                errorMessageLength = (ulong)this.errorMessageBuffer.Length;
                validatorReturnValue = this.validator.ValidateFile(
                    cziFilePath,
                    (nint)pOutputBuffer,
                    ref outputBufferSize,
                    (nint)pErrorBuffer,
                    ref errorMessageLength);

                if (validatorReturnValue == Result.BufferTooSmall &&
                    attempt == 1 &&
                    outputBufferSize < int.MaxValue)
                {
                    // Retry with resized buffer
                    this.outputStringBuffer = new byte[(int)outputBufferSize];
                }
                else
                {
                    break;
                }
            }
        }

        // Convert buffers to strings
        string? jsonOutput = null;
        string? errorMessage = null;

        if (outputBufferSize > 0 && validatorReturnValue == Result.Success)
        {
            jsonOutput = GetUtf8StringFromBuffer(this.outputStringBuffer);
        }

        if (errorMessageLength > 0)
        {
            errorMessage = GetUtf8StringFromBuffer(this.errorMessageBuffer);
        }

        return ParseJsonOrThrow(
            cziFilePath,
            validatorReturnValue,
            jsonOutput,
            errorMessage);
    }

    private static unsafe FileResult ParseJsonOrThrow(
        string cziFilePath,
        Result validatorReturnValue,
        string? jsonOutput,
        string? errorMessage)
    {
        // Handle different result codes
        return validatorReturnValue switch
        {
            Result.Success =>
                FileResultDto
                .FromJson(jsonOutput ?? string.Empty, errorMessage)
                .ToResultFor(cziFilePath),
            Result.FileAccessError =>
                throw new IOException(
                    $"File access error: Could not open or read the CZI file {cziFilePath}. {errorMessage}"),
            Result.UnsupportedCheck =>
                throw new InvalidOperationException(
                    "One or more requested checks are not available. " + errorMessage),
            _ =>
                throw new InvalidOperationException(
                    $"Validation failed with unknown error code {validatorReturnValue}. {errorMessage}")
        };
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

    /// <summary>
    /// Disposes the CziChecker and releases native resources.
    /// </summary>
    public void Dispose()
    {
        this.validator.Dispose();
    }
}