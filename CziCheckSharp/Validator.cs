// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Runtime.InteropServices;

/// <summary>
/// Wrapper for a native CZICheck validator instance.
/// </summary>
internal sealed class Validator : SafeHandle
{
    public enum Result
    {
        Success = 0,
        BufferTooSmall = 1,
        FileAccessError = 2,
        InvalidPointer = 3,
        UnsupportedCheck = 4,
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Validator"/> class.
    /// </summary>
    /// <param name="configuration">Configuration options for CZI validation.</param>
    /// <exception cref="InvalidOperationException">Thrown when validator creation fails.</exception>
    public Validator(Configuration configuration)
        : base(nint.Zero, ownsHandle: true)
    {
        ArgumentNullException.ThrowIfNull(configuration);
        
        this.SetHandle(NativeMethods.CreateValidator(
            checksBitmask: (ulong)configuration.Checks,
            maxFindings: configuration.MaxFindings,
            laxParsing: configuration.LaxParsing,
            ignoreSizeM: configuration.IgnoreSizeM));
        
        if (this.IsInvalid)
        {
            throw new InvalidOperationException("Failed to create validator. Invalid configuration parameters.");
        }
    }

    /// <summary>
    /// Gets a value indicating whether the handle value is invalid.
    /// </summary>
    public override bool IsInvalid => this.handle == nint.Zero;

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
    public Result ValidateFile(
        string inputPath,
        nint jsonBuffer,
        ref ulong jsonBufferSize,
        nint errorMessage,
        ref ulong errorMessageLength)
    {
        bool addedRef = false;
        try
        {
            this.DangerousAddRef(ref addedRef);
            
            return (Result)NativeMethods.ValidateFile(
                validator: this.handle,
                inputPath: inputPath,
                jsonBuffer: jsonBuffer,
                jsonBufferSize: ref jsonBufferSize,
                errorMessage: errorMessage,
                errorMessageLength: ref errorMessageLength);
        }
        finally
        {
            if (addedRef)
            {
                this.DangerousRelease();
            }
        }
    }

    /// <summary>
    /// Releases the native validator handle.
    /// </summary>
    /// <returns>true if the handle is released successfully; otherwise, false.</returns>
    protected override bool ReleaseHandle()
    {
        NativeMethods.DestroyValidator(this.handle);
        return true;
    }
}