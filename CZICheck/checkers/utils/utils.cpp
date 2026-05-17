// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "utils.h"

bool Utils::CompareValidDimensions(const libCZI::IDimCoordinate& coord1, const libCZI::IDimCoordinate& coord2)
{
    for (int dimIndex = static_cast<int>(libCZI::DimensionIndex::MinDim); dimIndex <= static_cast<int>(libCZI::DimensionIndex::MaxDim); dimIndex++)
    {
        const auto dim = static_cast<libCZI::DimensionIndex>(dimIndex);
        if (coord1.IsValid(dim))
        {
            if (!coord2.IsValid(dim))
            {
                return false;
            }

            int pos1, pos2;
            coord1.TryGetPosition(dim, &pos1);
            coord2.TryGetPosition(dim, &pos2);
            if (pos1 != pos2)
            {
                return false;
            }
        }
    }

    return true;
}
