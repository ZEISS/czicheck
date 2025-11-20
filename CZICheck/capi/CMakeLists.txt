# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
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

include(FetchContent)

set (TARGET_NAME libczicheckc)

# Source files for the C API library (reusing most of the CZICheck sources)
set(CAPI_SRCFILES 
    capi.h
    capi.cpp
    ../checkers/checkerbase.h
    ../checkers/checkerbase.cpp
    ../checkers/checkerConsistentCoordinates.cpp
    ../checkers/checkerConsistentCoordinates.h
    ../checkers/checkerSubBlkDirPositions.cpp
    ../checkers/checkerSubBlkDirPositions.h
    ../checkers/checkerSubBlkSegmentsValid.cpp
    ../checkers/checkerSubBlkSegmentsValid.h
    ../checkers/checkerDuplicateCoordinates.h
    ../checkers/checkerDuplicateCoordinates.cpp
    ../checkers/checkerBenabled.h
    ../checkers/checkerBenabled.cpp
    ../checkers/checkerSamePixeltypePerChannel.h
    ../checkers/checkerSamePixeltypePerChannel.cpp
    ../checkers/checkerXmlMetadataXsdValidation.h
    ../checkers/checkerXmlMetadataXsdValidation.cpp
    ../checkers/checkerXmlMetadataXsdSchema.h
    ../checkers/checkerXmlMetadataXsdSchema.cpp
    ../checkers/checkerMissingMindex.h
    ../checkers/checkerMissingMindex.cpp
    ../checkers/checkerPlanesStartIndices.h
    ../checkers/checkerPlanesStartIndices.cpp
    ../checkers/checkerConsecutivePlaneIndices.h
    ../checkers/checkerConsecutivePlaneIndices.cpp
    ../checkers/checkerXmlBasicMetadataValidation.h
    ../checkers/checkerXmlBasicMetadataValidation.cpp
    ../checkers/checkerOverlappingScenes.h
    ../checkers/checkerOverlappingScenes.cpp
    ../checkers/checkerSubBlkBitmapValid.h
    ../checkers/checkerSubBlkBitmapValid.cpp
    ../checkers/checkerTopographyApplianceValidation.h
    ../checkers/checkerTopographyApplianceValidation.cpp
    ../checkerfactory.cpp
    ../checkerfactory.h
    ../checks.h
    ../checks.cpp
    ../cmdlineoptions.cpp
    ../cmdlineoptions.h
    ../consoleio.cpp
    ../consoleio.h
    ../IChecker.h
    ../IResultGatherer.h
    ../resultgatherer.cpp
    ../resultgatherer.h
    ../resultgathererjson.h
    ../resultgathererjson.cpp
    ../resultgathererxml.h
    ../resultgathererxml.cpp
    ../resultgathererfactory.h
    ../resultgathererfactory.cpp
    ../runchecks.cpp
    ../runchecks.h
    ../utils.h
    ../utils.cpp
    ../inc_libCZI.h
)

add_library (${TARGET_NAME} SHARED ${CAPI_SRCFILES})

if(WIN32)
  set(CZICHECK_RCVersion ${CZICheck_VERSION_MAJOR},${CZICheck_VERSION_MINOR},${CZICheck_VERSION_PATCH},0)
  set(CZICHECK_RCVersion_STR ${CZICheck_VERSION_MAJOR},${CZICheck_VERSION_MINOR},${CZICheck_VERSION_PATCH},0)

  configure_file (
    CZICheck_Resource.rc.in CZICheck_Resource.rc @ONLY
  )

  add_library(${TARGET_NAME}_Version OBJECT "${CMAKE_CURRENT_BINARY_DIR}/CZICheck_Resource.rc")
  set_property(TARGET ${TARGET_NAME}_Version PROPERTY INCLUDE_DIRECTORIES "")
  target_link_libraries(${TARGET_NAME} PRIVATE ${TARGET_NAME}_Version)

  target_compile_options(${TARGET_NAME} PUBLIC "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  target_compile_options(${TARGET_NAME} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)

include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(${TARGET_NAME}
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

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/CZICheck_Config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CZICheck_Config.h")

target_compile_definitions(${TARGET_NAME} PRIVATE _LIBCZISTATICLIB)

# Link CLI11 (required by cmdlineoptions.cpp even though we don't use parsing in capi)
# CLI11 is already made available by the parent CMakeLists.txt via FetchContent
target_link_libraries(${TARGET_NAME} PRIVATE CLI11::CLI11)


# Link libraries
if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
  if(TARGET libczi::libCZI)
      add_library(libczi::libCZI_selected ALIAS libczi::libCZI)
  elseif(TARGET libczi::libCZIStatic)
      add_library(libczi::libCZI_selected ALIAS libczi::libCZIStatic)
  else()
      message(FATAL_ERROR "No suitable libCZI target found (neither shared nor static).")
  endif()
  target_link_libraries(${TARGET_NAME} PRIVATE libczi::libCZI_selected)
else()
  target_link_libraries(${TARGET_NAME} PRIVATE libCZIStatic)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE pugixml)

if (XercesC_FOUND)
  if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
    target_link_libraries(${TARGET_NAME} PRIVATE XercesC::XercesC)
  else()
    target_link_libraries(${TARGET_NAME} PRIVATE ${XercesC_LIBRARY})
  endif()
endif()

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
  get_target_property(_libczi_includes libczi::libCZI_selected INTERFACE_INCLUDE_DIRECTORIES)
  set(_libczi_include_with_subdir "${_libczi_includes}/libCZI")
  target_include_directories(${TARGET_NAME} PRIVATE ${_libczi_include_with_subdir})
else()
  target_include_directories(${TARGET_NAME} PRIVATE ${LIBCZI_INCLUDE_DIR})
endif()

target_include_directories(${TARGET_NAME}
  PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
