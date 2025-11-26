# SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

message (STATUS "[${PROJECT_NAME}] Processing ${CMAKE_CURRENT_LIST_FILE}")

function(embed_resource resource_file_name source_file_name variable_name)
    file(SIZE ${resource_file_name} file_size)
    file(READ ${resource_file_name} hex_content HEX)
    string(REPEAT "[0-9a-f]" 32 column_pattern)
    string(REGEX REPLACE "(${column_pattern})" "\\1\n" content "${hex_content}")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " content "${content}")
    string(REGEX REPLACE ", $" "" content "${content}")
    set(array_definition "static const unsigned char ${variable_name}[${file_size}] =\n{\n${content}\n};")
    set(source "// Auto generated file.\n${array_definition}\n")
    file(WRITE "${source_file_name}" "${source}")
endfunction()


if (XercesC_FOUND)
    set(CZICheck_XercesCAvailable 1)
    message("XercesC library available -> version: ${XercesC_VERSION}")
    embed_resource( "${CMAKE_CURRENT_SOURCE_DIR}/../checkers/schema/ZEN/flatten/ImageMetadata.xsd" "${CMAKE_CURRENT_BINARY_DIR}/ImageMetadataFlattened.frag" zenFlattenedCompleteXsd)
else()
    set(CZICheck_XercesCAvailable 0)
    message("XercesC library not found, the checker 'xmlmetadataschema' will **not** be available in capi")
endif()

# Source files for the C API library (reusing most of the CZICheck sources)
set(CAPI_SRCFILES 
    capi.h
    capi.cpp)

add_library(${CZICHECK_SHARED_LIB} SHARED ${CAPI_SRCFILES})

if(WIN32)
  set(CZICHECK_RCVersion ${CZICheck_VERSION_MAJOR},${CZICheck_VERSION_MINOR},${CZICheck_VERSION_PATCH},0)

  configure_file(CZICheck_Resource.rc.in CZICheck_Resource.rc @ONLY)
  target_sources(${CZICHECK_SHARED_LIB} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/CZICheck_Resource.rc")

  target_compile_options(${CZICHECK_SHARED_LIB} PUBLIC "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  target_compile_options(${CZICHECK_SHARED_LIB} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

set_target_properties(${CZICHECK_SHARED_LIB} PROPERTIES PREFIX "")
set_target_properties(${CZICHECK_SHARED_LIB} PROPERTIES CXX_STANDARD 17)

include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(${CZICHECK_SHARED_LIB}
    BASE_NAME CAPI)

# prepare the CZICheck_Config.h file - which is then to be found in the binary directory ${CMAKE_CURRENT_BINARY_DIR}
IF(WIN32)
  set(CZICheck_UNIX_ENVIRONMENT 0)
  set(CZICheck_WIN32_ENVIRONMENT 1)
ENDIF()
IF(UNIX)
  set(CZICheck_UNIX_ENVIRONMENT 1)
  set(CZICheck_WIN32_ENVIRONMENT 0)
ENDIF()

# Link CLI11 (required by cmdlineoptions.cpp even though we don't use parsing in capi)
# CLI11 is already made available by the parent CMakeLists.txt via FetchContent
target_link_libraries(${CZICHECK_SHARED_LIB} PUBLIC ${CZICHECK_STATIC_LIB})
target_include_directories(${CZICHECK_SHARED_LIB} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
