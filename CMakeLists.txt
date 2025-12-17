# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

option(CZICHECK_ENABLE_VCPKG_BOOTSTRAP
       "Automatically fetch and bootstrap vcpkg if no toolchain is defined" ON)

# Include the bootstrap script if the option is enabled
if(CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
    include(modules/bootstrapVcpkg.cmake)
endif()

project(CZICheck 
      VERSION 0.7.0
      HOMEPAGE_URL "https://github.com/ZEISS/czicheck"
      DESCRIPTION "CZICheck is a validator for CZI-documents")



if(WIN32)
  # use "static C-runtime" -> https://stackoverflow.com/questions/14172856/compile-with-mt-instead-of-md-using-cmake
  # Note: this requires CMAKE version 3.15+
  cmake_policy(SET CMP0091 NEW)
  if(NOT CZICHECK_ENABLE_VCPKG_BOOTSTRAP OR VCPKG_TARGET_TRIPLET MATCHES "-static$")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

endif(WIN32)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules" ${CMAKE_MODULE_PATH})

# Enable libCZI stream support (curl for HTTP/HTTPS URLs)
set(LIBCZI_BUILD_CURL_BASED_STREAM ON CACHE BOOL "Enable curl-based streams" FORCE)
set(LIBCZI_BUILD_PREFER_EXTERNALPACKAGE_LIBCURL ON CACHE BOOL "Use external libcurl package" FORCE)

if(NOT CZICHECK_ENABLE_VCPKG_BOOTSTRAP)
  include("${CMAKE_SOURCE_DIR}/modules/libCZI.cmake")
  include("${CMAKE_SOURCE_DIR}/modules/rapidJSON.cmake")
  FetchContent_GetProperties(libCZI)
  set(LIBCZI_INCLUDE_DIR "${libczi_SOURCE_DIR}/Src/libCZI")
endif()

include("${CMAKE_SOURCE_DIR}/modules/pugiXML.cmake")

# This option enables/disables the tests for the CZICheck-application. If enabled, the test
#  runs with a ctest run (and test-data is being downloaded)
option(CZICHECK_BUILD_TESTS "Run the tests-suite with CTest (requires external files)" OFF)

if (CZICHECK_BUILD_TESTS)
 enable_testing()
endif()

add_subdirectory(CZICheck)


