# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

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

if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
  find_package(XercesC CONFIG REQUIRED)
else()
  find_package(XercesC QUIET)
endif()

if (XercesC_FOUND)
    set(CZICheck_XercesCAvailable 1)
    message("XercesC library available -> version: ${XercesC_VERSION}")
else()
    set(CZICheck_XercesCAvailable 0)
endif()

if (XercesC_FOUND)
    message("XercesC library found, the checker 'xmlmetadataschema' will be available")
    embed_resource( "${CMAKE_CURRENT_SOURCE_DIR}/checkers/schema/ZEN/flatten/ImageMetadata.xsd" "${CMAKE_CURRENT_BINARY_DIR}/ImageMetadataFlattened.frag" zenFlattenedCompleteXsd)
else()
    message("XercesC library not found, the checker 'xmlmetadataschema' will **not** be available")
endif()

include(FetchContent)
FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG        v2.3.2
)

FetchContent_MakeAvailable(cli11)

if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
  find_package(libCZI CONFIG REQUIRED)
endif()

set(CZICHECKSRCFILES 
"checkers/checkerbase.h"
"checkers/checkerbase.cpp"
"checkers/checkerConsistentCoordinates.cpp"
"checkers/checkerConsistentCoordinates.h"
"checkers/checkerSubBlkDirPositions.cpp"
"checkers/checkerSubBlkDirPositions.h"
"checkers/checkerSubBlkSegmentsValid.cpp"
"checkers/checkerSubBlkSegmentsValid.h"
"checkers/checkerDuplicateCoordinates.h"
"checkers/checkerDuplicateCoordinates.cpp"
"checkers/checkerBenabled.h"
"checkers/checkerBenabled.cpp"
"checkers/checkerSamePixeltypePerChannel.h"
"checkers/checkerSamePixeltypePerChannel.cpp"
"checkers/checkerXmlMetadataXsdValidation.h"
"checkers/checkerXmlMetadataXsdValidation.cpp"
"checkers/checkerXmlMetadataXsdSchema.h"
"checkers/checkerXmlMetadataXsdSchema.cpp"
"checkers/checkerMissingMindex.h"
"checkers/checkerMissingMindex.cpp"
"checkers/checkerPlanesStartIndices.h"
"checkers/checkerPlanesStartIndices.cpp"
"checkers/checkerConsecutivePlaneIndices.h"
"checkers/checkerConsecutivePlaneIndices.cpp"
"checkers/checkerXmlBasicMetadataValidation.h"
"checkers/checkerXmlBasicMetadataValidation.cpp"
"checkers/checkerOverlappingScenes.h"
"checkers/checkerOverlappingScenes.cpp"
"checkers/checkerSubBlkBitmapValid.h"
"checkers/checkerSubBlkBitmapValid.cpp"
"checkers/checkerTopographyApplianceValidation.h"
"checkers/checkerTopographyApplianceValidation.cpp"
"checkerfactory.cpp"
"checkerfactory.h"
"checks.h"
"checks.cpp"
"CMakeLists.txt"
"cmdlineoptions.cpp"
"cmdlineoptions.h"
"consoleio.cpp"
"consoleio.h"
"CZICheck.cpp"
"IChecker.h"
"IResultGatherer.h"
"resultgatherer.cpp"
"resultgatherer.h"
"resultgathererjson.h"
"resultgathererjson.cpp"
"resultgathererxml.h"
"resultgathererxml.cpp"
"resultgathererfactory.h"
"resultgathererfactory.cpp"
"runchecks.cpp"
"runchecks.h"
"utils.h"
"utils.cpp"
"inc_libCZI.h"
)

add_executable(CZICheck ${CZICHECKSRCFILES} )

set_target_properties(CZICheck PROPERTIES CXX_STANDARD 17)
target_compile_definitions(CZICheck PRIVATE _LIBCZISTATICLIB)

target_link_libraries(CZICheck PRIVATE CLI11::CLI11)

target_include_directories(CZICheck PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
  if(TARGET libczi::libCZI)
      add_library(libczi::libCZI_selected ALIAS libczi::libCZI)
  elseif(TARGET libczi::libCZIStatic)
      add_library(libczi::libCZI_selected ALIAS libczi::libCZIStatic)
  else()
      message(FATAL_ERROR "No suitable libCZI target found (neither shared nor static).")
  endif()
  target_link_libraries(CZICheck PRIVATE libczi::libCZI_selected)
else()
  target_link_libraries(CZICheck PRIVATE libCZIStatic)
endif()
if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
  get_target_property(_libczi_includes libczi::libCZI_selected INTERFACE_INCLUDE_DIRECTORIES)
  set(_libczi_include_with_subdir "${_libczi_includes}/libCZI")
  target_include_directories(CZICheck PRIVATE ${_libczi_include_with_subdir})
else()
  target_include_directories(CZICheck PRIVATE ${LIBCZI_INCLUDE_DIR})
endif()
target_link_libraries(CZICheck PRIVATE pugixml)

target_include_directories(CZICheck
  PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

if (XercesC_FOUND)
  if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
    target_link_libraries(CZICheck PRIVATE XercesC::XercesC)
  else()
    target_link_libraries(CZICheck PRIVATE ${XercesC_LIBRARY})
  endif()
endif()
target_include_directories(CZICheck PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if (XercesC_FOUND AND NOT CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
    target_include_directories(CZICheck PRIVATE ${XercesC_INCLUDE_DIR})
endif()

IF(UNIX)
  # seems to be problem with glibc I'd reckon -> https://stackoverflow.com/questions/51584960/stdcall-once-throws-stdsystem-error-unknown-error-1
  target_link_libraries(CZICheck PUBLIC pthread)
ENDIF(UNIX)

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
  "${CMAKE_CURRENT_BINARY_DIR}/CZICheck_Config.h"
  )

# checking platform -> https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/Checking-Platform
#                   -> https://stackoverflow.com/questions/9160335/os-specific-instructions-in-cmake-how-to

install(TARGETS CZICheck RUNTIME  DESTINATION bin)

if (CZICHECK_BUILD_TESTS)
  include(ExternalData)

  # this instructs CMake to pull test-data from the URLs given here. The concept used here is that of "content links",
  #  and is described here -> https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html#managing-test-data
  #  or https://crascit.com/2015/04/03/handling-binary-assets-and-test-data-with-cmake/
  list(APPEND ExternalData_URL_TEMPLATES
    "https://libczirwtestdata.z13.web.core.windows.net/CZICheckSamples/%(algo)/%(hash)"
    "https://github.com/ptahmose/libCZI_testdata/raw/main/%(algo)/%(hash)"
   )

  find_package(Python QUIET)

  if (Python_FOUND)
    # with this "-r"-arguments, we instruct CZICheckRunTests.py-script to replace a CZI (listed in the test-list)
    #  by the filename of the corresponding file pulled down by CMake's test-data management
    ExternalData_Add_Test(Test_CZICheck
        NAME Test-CZICheck 
        COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/CZICheckRunTests.py 
          --executable $<TARGET_FILE:CZICheck> 
          --knowngoodresultspath ${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples 
          --test_list ${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/TestCasesLists.txt
          -r differentpixeltypeinchannel.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/differentpixeltypeinchannel.czi}
          -r duplicate_coordinates.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/duplicate_coordinates.czi}
          -r inconsistent_coordinates.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/inconsistent_coordinates.czi}
          -r layer_0_subblocks_with_no_m_index.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/layer_0_subblocks_with_no_m_index.czi}
          -r negative_plane_start_index.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/negative_plane_start_index.czi}
          -r pixeltype_mismatch_between_metadata_and_subblocks.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/pixeltype_mismatch_between_metadata_and_subblocks.czi}
          -r positive_plane_start_index.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/positive_plane_start_index.czi}
          -r sparse_planes.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/sparse_planes.czi}
          -r overlapping_scenes.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/overlapping_scenes.czi}
          -r jpgxrcompressed_inconsistent_size.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/jpgxrcompressed_inconsistent_size.czi}
          -r jpgxrcompressed_inconsistent_pixeltype.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/jpgxrcompressed_inconsistent_pixeltype.czi}
          -r edf-missing-texture.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/edf-missing-texture.czi}
          -r edf-superfluous.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/edf-superfluous.czi}
          -r edf-superfluous-missing-channel-subblock.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/edf-superfluous-missing-channel-subblock.czi}
          -r invalid_componentbitcount.czi=DATA{${CMAKE_CURRENT_SOURCE_DIR}/../Test/CZICheckSamples/invalid_componentbitcount.czi}
    )

    # Add a build target to populate the real data.
    ExternalData_Add_Target(Test_CZICheck SHOW_PROGRESS OFF)
  else()
    message("Python not found, CZICheck-Test will not run.")
  endif()
endif()

# Add the C API library subdirectory
add_subdirectory(capi)
