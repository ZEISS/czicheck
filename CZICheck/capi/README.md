# CZICheck C API

This directory contains the C API for CZICheck, which provides a reusable library interface for validating CZI files programmatically.

## Library Name

`libczicheckc` - A shared library providing C API access to CZICheck validation functionality.

## API Overview

The C API provides the following main functions:

### Validator Lifecycle

- **`CreateValidator()`** - Creates a validator instance with specified checks and options
- **`ValidateFile()`** - Validates a CZI file and returns JSON results
- **`DestroyValidator()`** - Destroys a validator instance

### Version Information

- **`GetLibVersion()`** - Gets major, minor, and patch version numbers
- **`GetLibVersionString()`** - Gets version as a formatted string

## Check Selection

Checks are specified using a bitmask with the following constants:

```c
#define CZICHECK_SUBBLOCK_DIR_POSITIONS                 0x0001ULL
#define CZICHECK_SUBBLOCK_SEGMENT_VALID                 0x0002ULL
#define CZICHECK_CONSISTENT_SUBBLOCK_COORDINATES        0x0004ULL
#define CZICHECK_DUPLICATE_SUBBLOCK_COORDINATES         0x0008ULL
#define CZICHECK_BENABLED_DOCUMENT                      0x0010ULL
#define CZICHECK_SAME_PIXELTYPE_PER_CHANNEL             0x0020ULL
#define CZICHECK_PLANES_START_AT_ZERO                   0x0040ULL
#define CZICHECK_PLANES_CONSECUTIVE                     0x0080ULL
#define CZICHECK_SUBBLOCKS_HAVE_MINDEX                  0x0100ULL
#define CZICHECK_BASIC_METADATA_VALIDATION              0x0200ULL
#define CZICHECK_XML_METADATA_SCHEMA_VALIDATION         0x0400ULL
#define CZICHECK_OVERLAPPING_SCENES_LAYER0              0x0800ULL
#define CZICHECK_SUBBLOCK_BITMAP_VALID                  0x1000ULL
#define CZICHECK_CONSISTENT_MINDEX                      0x2000ULL
#define CZICHECK_ATTACHMENT_DIR_POSITIONS               0x4000ULL
#define CZICHECK_APPLIANCE_METADATA_TOPOGRAPHY_VALID    0x8000ULL

// Convenience macros
#define CZICHECK_ALL                                    0xFFFFULL
#define CZICHECK_ALL_DEFAULT (CZICHECK_ALL & ~CZICHECK_XML_METADATA_SCHEMA_VALIDATION & ~CZICHECK_SUBBLOCK_BITMAP_VALID)
```

## Usage Example

```c
#include "capi.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Create a validator with default checks
    void* validator = CreateValidator(
        CZICHECK_ALL_DEFAULT,  // checks_bitmask
        -1,                     // max_findings (-1 = unlimited)
        false,                  // lax_parsing
        false                   // ignore_sizem
    );
    
    if (validator == NULL) {
        fprintf(stderr, "Failed to create validator\n");
        return 1;
    }
    
    // Query required buffer size
    uint64_t json_buffer_size = 0;
    int result = ValidateFile(validator, "test.czi", NULL, &json_buffer_size, NULL, NULL);
    
    if (result == 1) {
        // Allocate buffer and validate
        char* json_buffer = malloc(json_buffer_size);
        char error_buffer[1024];
        size_t error_length = sizeof(error_buffer);
        
        result = ValidateFile(validator, "test.czi", json_buffer, &json_buffer_size,
                             error_buffer, &error_length);
        
        if (result == 0) {
            // Success - process JSON results
            printf("Validation results:\n%s\n", json_buffer);
        } else if (result == 2) {
            // File access error
            fprintf(stderr, "Error: %s\n", error_buffer);
        }
        
        free(json_buffer);
    }
    
    // Clean up
    DestroyValidator(validator);
    return 0;
}
```

## Return Codes

### ValidateFile() Return Values:
- `0` - Success, validation completed, JSON results in buffer
- `1` - Buffer too small, required size written to `json_buffer_size`
- `2` - File access error (error details in `error_message` if provided)
- `3` - Invalid validator pointer or parameters

## Features

- **Reusable validators**: Create once, validate multiple files
- **Thread-safe instances**: Different validator instances can be used concurrently from different threads
- **UTF-8 encoding**: All strings (file paths, JSON output, error messages) use UTF-8 encoding
- **JSON output**: Results always returned as JSON with detailed findings
- **Configurable checks**: Enable/disable specific validation checks via bitmask
- **Memory management**: Caller-controlled buffer allocation using size-query pattern

## Build Requirements

- CMake 3.15+
- C++17 compiler
- libCZI
- RapidJSON (for JSON output)
- pugiXML
- CLI11
- XercesC (optional, for XML schema validation)

## Notes

- The validator automatically initializes COM on Windows and XercesC if available
- XML schema validation check requires XercesC to be compiled in
- Print details is always enabled (as per requirements)
- Output format is always JSON (not configurable via API)
- No progress reporting in the C API
