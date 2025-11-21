// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp.Tests;

using AwesomeAssertions;

/// <summary>
/// Tests for <see cref="CziChecker"/>.
/// </summary>
public class CziCheckerTests
{
    [Fact]
    public void GetVersion_ReturnsExpected()
    {
        _ = CziChecker.GetCziCheckVersion().Should().Be("0.6.5");
    }
}