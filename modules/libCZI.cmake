# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

include(FetchContent)

FetchContent_Declare(
  libCZI
  GIT_REPOSITORY https://github.com/ZEISS/libczi.git
  GIT_TAG        0bcf442562dde287c2ef14c86bee56cba0387f6f # v0.73.0
)

if(NOT libCZI_POPULATED)
  message(STATUS "Fetching libCZI")
  FetchContent_Populate(libCZI)

  set(LIBCZI_BUILD_CZICMD OFF CACHE BOOL "" FORCE)
  set(LIBCZI_BUILD_DYNLIB OFF CACHE BOOL "" FORCE)
  set(LIBCZI_BUILD_UNITTESTS OFF CACHE BOOL "" FORCE)

  add_subdirectory(${libczi_SOURCE_DIR} ${libczi_BINARY_DIR})
endif()