# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

project(CZICheck 
      VERSION 0.6.3
      HOMEPAGE_URL "https://github.com/ZEISS/czicheck"
      DESCRIPTION "CZICheck is a validator for CZI-documents")

if(WIN32)
  # use "static C-runtime" -> https://stackoverflow.com/questions/14172856/compile-with-mt-instead-of-md-using-cmake
  # Note: this requires CMAKE version 3.15+
  cmake_policy(SET CMP0091 NEW)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(WIN32)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules" ${CMAKE_MODULE_PATH})

include("${CMAKE_SOURCE_DIR}/modules/libCZI.cmake")
include("${CMAKE_SOURCE_DIR}/modules/rapidJSON.cmake")
include("${CMAKE_SOURCE_DIR}/modules/pugiXML.cmake")
FetchContent_GetProperties(libCZI)
set(LIBCZI_INCLUDE_DIR "${libczi_SOURCE_DIR}/Src/libCZI")

# This option enables/disables the tests for the CZICheck-application. If enabled, the test
#  runs with a ctest run (and test-data is being downloaded)
option(CZICHECK_BUILD_TESTS "Run the tests-suite with CTest (requires external files)" OFF)

if (CZICHECK_BUILD_TESTS)
 enable_testing()
endif()

add_subdirectory(CZICheck)


