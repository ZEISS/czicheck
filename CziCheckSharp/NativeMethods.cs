// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

using System.Runtime.InteropServices;

/// <summary>
/// P/Invoke declarations for libczicheckc native library.
/// </summary>
internal static partial class NativeMethods
{
    private const string LibraryName = "libczicheckc";

    /// <summary>
    /// Creates a new validator for use in ValidateFile().
    /// </summary>
    /// <param name="checksBitmask">A bitmask of CZICHECK_* constants specifying which checks to perform.</param>
    /// <param name="maxFindings">Maximum number of findings to report per check (-1 for unlimited).</param>
    /// <param name="laxParsing">If true, enables lax parsing mode (tolerates some CZI format violations).</param>
    /// <param name="ignoreSizeM">If true, ignores size M dimension for pyramid subblocks.</param>
    /// <returns>A new validator instance for the specified configuration, or NULL if parameters are invalid.</returns>
    [LibraryImport(LibraryName)]
    internal static partial nint CreateValidator(
        ulong checksBitmask,
        int maxFindings,
        [MarshalAs(UnmanagedType.U1)] bool laxParsing,
        [MarshalAs(UnmanagedType.U1)] bool ignoreSizeM);

    /// <summary>
    /// Validates a single CZI file with the specified validator.
    /// </summary>
    /// <param name="validator">A validator pointer obtained with CreateValidator().</param>
    /// <param name="inputPath">The UTF-8 encoded, null-terminated path of the CZI file to validate.</param>
    /// <param name="jsonBuffer">A character buffer to receive the UTF-8 encoded JSON validation results.</param>
    /// <param name="jsonBufferSize">On input: the size of json_buffer in bytes. On output: the size of the JSON result (including null terminator), or the required size if the buffer was too small.</param>
    /// <param name="errorMessage">A character buffer to receive UTF-8 encoded error messages for non-validation errors.</param>
    /// <param name="errorMessageLength">On input: the size of error_message buffer. On output: the length of the error message written (if any).</param>
    /// <returns>
    /// Zero (0) in case of success (validation completed, results in json_buffer).
    /// Return value 1: json_buffer_size too small (required size written to json_buffer_size).
    /// Return value 2: File access error (error details in error_message if provided).
    /// Return value 3: Invalid validator pointer.
    /// </returns>
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    internal static partial int ValidateFile(
        nint validator,
        string inputPath,
        nint jsonBuffer,
        ref ulong jsonBufferSize,
        nint errorMessage,
        ref ulong errorMessageLength);

    /// <summary>
    /// Destroys a validator after use.
    /// </summary>
    /// <param name="validator">A validator pointer obtained with CreateValidator().</param>
    [LibraryImport(LibraryName)]
    internal static partial void DestroyValidator(nint validator);

    /// <summary>
    /// Gets the version number of CZICheck.
    /// </summary>
    /// <param name="major">Set to the major version.</param>
    /// <param name="minor">Set to the minor version.</param>
    /// <param name="patch">Set to the patch version.</param>
    [LibraryImport(LibraryName)]
    internal static partial void GetLibVersion(out int major, out int minor, out int patch);

    /// <summary>
    /// Gets a string containing the version number of CZICheck (a null-terminated string in UTF-8 encoding).
    /// </summary>
    /// <param name="buffer">Pointer to a buffer (which size is stated with size).</param>
    /// <param name="size">Pointer to a uint64 which on input contains the size of the buffer, and on output (with return value false) the required size of the buffer.</param>
    /// <returns>True if the buffer size was sufficient (and in this case the text is copied to the buffer); false otherwise (and in this case the required size is written to *size).</returns>
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [return: MarshalAs(UnmanagedType.U1)]
    internal static partial bool GetLibVersionString(nint buffer, ref ulong size);
}