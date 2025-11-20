// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

/// <summary>
/// Wrapper for a native CZICheck validator instance.
/// </summary>
internal sealed class Validator : IDisposable
{
    private nint handle;

    /// <summary>
    /// Initializes a new instance of the <see cref="Validator"/> class.
    /// </summary>
    /// <param name="configuration">Configuration options for CZI validation.</param>
    /// <exception cref="InvalidOperationException">Thrown when validator creation fails.</exception>
    public Validator(Configuration configuration)
    {
        ArgumentNullException.ThrowIfNull(configuration);
        
        this.handle = NativeMethods.CreateValidator(
            checksBitmask: (ulong)configuration.Checks,
            maxFindings: configuration.MaxFindings,
            laxParsing: configuration.LaxParsing,
            ignoreSizeM: configuration.IgnoreSizeM);
        
        if (this.handle == nint.Zero)
        {
            throw new InvalidOperationException("Failed to create validator. Invalid configuration parameters.");
        }
    }

    public bool IsDisposed => this.handle == nint.Zero;

    /// <summary>
    /// Validates a CZI file.
    /// </summary>
    /// <param name="inputPath">The path to the CZI file to validate.</param>
    /// <param name="jsonBuffer">Pointer to the JSON output buffer.</param>
    /// <param name="jsonBufferSize">On input: size of buffer. On output: size needed/written.</param>
    /// <param name="errorMessage">Pointer to the error message buffer.</param>
    /// <param name="errorMessageLength">On input: size of buffer. On output: length of error message.</param>
    /// <returns>Result code from validation.</returns>
    /// <exception cref="ObjectDisposedException">Thrown when the validator has been disposed.</exception>
    public int ValidateFile(
        string inputPath,
        nint jsonBuffer,
        ref ulong jsonBufferSize,
        nint errorMessage,
        ref ulong errorMessageLength)
    {
        ObjectDisposedException.ThrowIf(this.IsDisposed, this);

        return NativeMethods.ValidateFile(
            validator: this.handle,
            inputPath: inputPath,
            jsonBuffer: jsonBuffer,
            jsonBufferSize: ref jsonBufferSize,
            errorMessage: errorMessage,
            errorMessageLength: ref errorMessageLength);
    }

    /// <summary>
    /// Disposes the validator and releases native resources.
    /// </summary>
    public void Dispose()
    {
        if (!this.IsDisposed)
        {
            NativeMethods.DestroyValidator(this.handle);
            this.handle = nint.Zero;
        }
    }
}