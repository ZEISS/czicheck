// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>

class Utils
{
public:
    /// Compares the valid dimensions of the first coordinate (coord1) for equality with the
    /// second coordinate (coord2). If the in the second coordinate (coord2) all dimensions
    /// are valid which are valid in the first coordinate (coord1), and if the values for those
    /// dimensions are identical, then this function returns true. Otherwise, it returns false.
    /// \param coord1 The first coordinate to compare.
    /// \param coord2 The second coordinate to compare.
    /// \returns A boolean value indicating whether the valid dimensions of the first coordinate are identical 
    ///          to the second coordinate and the values for those dimensions are identical.
    static bool CompareValidDimensions(const libCZI::IDimCoordinate& coord1, const libCZI::IDimCoordinate& coord2);
};
