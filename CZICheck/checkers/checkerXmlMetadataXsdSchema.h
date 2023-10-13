// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <CZICheck_Config.h>

#if CZICHECK_XERCESC_AVAILABLE

#include <cstddef>

/// Gets the XSD-schema for the ZEN-XML-metadata. The returned pointer is
/// pointing to static memory and must not be freed. It is valid for the
/// whole runtime of the application.
///
/// \param [out] size If non-null, the size of the string (in bytes) is returned.
///
/// \returns    Pointer to the XSD-schema.
const char* GetZenCompleteXsd(size_t* size);

#endif
