// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <cstdint>

#include "capi_export.h"

// Bitmask constants for CZI validation checks
// Each bit corresponds to a specific check that can be performed on a CZI file
#define CZICHECK_HAS_VALID_SUBBLOCK_POSITIONS                   0x0001ULL
#define CZICHECK_HAS_VALID_SUBBLOCK_SEGMENTS                    0x0002ULL
#define CZICHECK_HAS_CONSISTENT_SUBBLOCK_DIMENSIONS             0x0004ULL
#define CZICHECK_HAS_NO_DUPLICATE_SUBBLOCK_COORDINATES          0x0008ULL
#define CZICHECK_DOES_NOT_USE_BINDEX                            0x0010ULL
#define CZICHECK_HAS_ONLY_ONE_PIXELTYPE_PER_CHANNEL             0x0020ULL
#define CZICHECK_HAS_PLANE_INDICES_STARTING_AT_ZERO             0x0040ULL
#define CZICHECK_HAS_CONSECUTIVE_PLANE_INDICES                  0x0080ULL
#define CZICHECK_ALL_SUBBLOCKS_HAVE_MINDEX                      0x0100ULL
#define CZICHECK_HAS_BASICALLY_VALID_METADATA                   0x0200ULL
#define CZICHECK_HAS_XML_SCHEMA_VALID_METADATA                  0x0400ULL
#define CZICHECK_HAS_NO_OVERLAPPING_SCENES_AT_SCALE1            0x0800ULL
#define CZICHECK_HAS_VALID_SUBBLOCK_BITMAPS                     0x1000ULL
#if FUTURE_CHECKS
#define CZICHECK_HAS_CONSISTENT_MINDICES                        0x2000ULL
#define CZICHECK_HAS_VALID_ATTACHMENT_DIR_POSITIONS             0x4000ULL
#endif
#define CZICHECK_HAS_VALID_APPLIANCE_METADATA_TOPOGRAPHY        0x8000ULL

// Convenience: All checks enabled
#define CZICHECK_ALL 0xFFFFULL

// Convenience: Default checks (excludes expensive/optional checks like schema validation and bitmap decoding)
#define CZICHECK_ALL_DEFAULT (CZICHECK_ALL & ~CZICHECK_HAS_XML_SCHEMA_VALID_METADATA & ~CZICHECK_HAS_VALID_SUBBLOCK_BITMAPS)

/**
 * Creates a new validator for use in ValidateFile().
 *
 * @param checks_bitmask     A bitmask of CZICHECK_* constants specifying which checks to perform.
 * @param max_findings       Maximum number of findings to report per check (-1 for unlimited).
 * @param lax_parsing        If true, enables lax parsing mode (tolerates some CZI format violations).
 * @param ignore_sizem       If true, ignores size M dimension for pyramid subblocks.
 *
 * @returns    A new validator instance for the specified configuration, or NULL if parameters are invalid.
 */
extern "C" CAPI_EXPORT void* CreateValidator(uint64_t checks_bitmask, int32_t max_findings, bool lax_parsing, bool ignore_sizem);

/**
 * Validates a single CZI file with the specified validator.
 *
 * The validation results are returned as JSON in UTF-8 encoding. The caller must provide a buffer
 * to receive the JSON result. If the buffer is too small, the function returns false and sets
 * json_buffer_size to the required size. The caller can then allocate a larger buffer and call again.
 *
 * @param validator          A validator pointer obtained with CreateValidator().
 * @param input_path         The UTF-8 encoded, null-terminated path of the CZI file to validate.
 * @param json_buffer        A character buffer to receive the UTF-8 encoded JSON validation results.
 * @param json_buffer_size   On input: the size of json_buffer in bytes. On output: the size of the JSON
 *                           result (including null terminator), or the required size if the buffer was too small.
 * @param error_message      A character buffer to receive UTF-8 encoded error messages for non-validation errors
 *                           (e.g., file not found, cannot open file). Can be NULL if not needed.
 * @param error_message_length  On input: the size of error_message buffer. On output: the length of the error
 *                           message written (if any). Can be NULL if error_message is NULL.
 *
 * @returns    Zero (0) in case of success (validation completed, results in json_buffer).
 *             Non-zero in case of failure (file access error, buffer too small, etc.).
 *             Return value 1: json_buffer_size too small (required size written to json_buffer_size).
 *             Return value 2: File access error (error details in error_message if provided).
 *             Return value 3: Invalid validator pointer.
 *             Return value 4: One or more requested checks are not available (error details in error_message if provided).
 */
extern "C" CAPI_EXPORT int ValidateFile(void* validator, const char* input_path, 
                                        char* json_buffer, uint64_t* json_buffer_size,
                                        char* error_message, uint64_t* error_message_length);

/**
 * Destroys a validator after use.
 *
 * @param validator   A validator pointer obtained with CreateValidator().
 */
extern "C" CAPI_EXPORT void DestroyValidator(void* validator);

/**
 * Gets the version number of CZICheck.
 *
 * @param major   Set to the major version.
 * @param minor   Set to the minor version.
 * @param patch   Set to the patch version.
 */
extern "C" CAPI_EXPORT void GetLibVersion(int32_t* major, int32_t* minor, int32_t* patch);

/**
 * Gets a string containing the version number of CZICheck (a null-terminated string in UTF-8 encoding).
 * The caller must pass in a pointer to a buffer and a pointer to a uint64, where the latter
 * must contain the length of the buffer in bytes. If the length of the buffer is sufficient,
 * then the string is copied into the buffer and the function returns true.
 * If the size of the buffer is insufficient, then the required size is written into '*size'
 * and the return value is false. In this case, the caller may increase the buffer size (to at
 * least the number given with the first call) and call again.
 *
 * @param buffer   Pointer to a buffer (which size is stated with size).
 * @param size     Pointer to a uint64 which on input contains the size of the buffer, and on output (with return value false)
 *                 the required size of the buffer.
 *
 * @returns    True if the buffer size was sufficient (and in this case the text is copied to the buffer); false otherwise (and in
 *             this case the required size is written to *size).
 */
extern "C" CAPI_EXPORT bool GetLibVersionString(char* buffer, uint64_t* size);
