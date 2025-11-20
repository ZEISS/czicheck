// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

/// <summary>
/// Configuration options for CZI check operations.
/// </summary>
public record class Configuration
{
    /// <summary>
    /// Specifies which checks to run. Default is <see cref="Checks.Default"/>.
    /// </summary>
    public Checks Checks { get; set; } = Checks.Default;

    /// <summary>
    /// Specifies how many findings are to be reported and printed (for every check).
    /// A negative number means 'no limit'. Default is -1.
    /// </summary>
    public int MaxFindings { get; set; } = -1;

    /// <summary>
    /// Specifies whether lax parsing for file opening is enabled.
    /// This option allows operation on some malformed CZIs which would
    /// otherwise not be analyzable at all. Default is false.
    /// </summary>
    public bool LaxParsing { get; set; } = false;

    /// <summary>
    /// Specifies whether to ignore the 'SizeM' field for pyramid subblocks.
    /// This option allows operation on some malformed CZIs which would
    /// otherwise not be analyzable at all. Default is false.
    /// </summary>
    public bool IgnoreSizeM { get; set; } = false;

    /// <summary>
    /// Timeout in milliseconds for the check operation.
    /// A negative number means no timeout. Default is 60000 (60 seconds).
    /// Note: This property is only used when running async operations and may not apply
    /// to synchronous native library calls.
    /// </summary>
    public int TimeoutMilliseconds { get; set; } = 60000;
}