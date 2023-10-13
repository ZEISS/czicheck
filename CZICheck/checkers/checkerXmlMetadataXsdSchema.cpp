// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerXmlMetadataXsdSchema.h"

#if CZICHECK_XERCESC_AVAILABLE

#include <ImageMetadataFlattened.frag>

const char* GetZenCompleteXsd(size_t* size)
{
    if (size != nullptr)
    {
        *size = sizeof(zenFlattenedCompleteXsd);
    }

    return reinterpret_cast<const char*>(zenFlattenedCompleteXsd);
}

#endif
