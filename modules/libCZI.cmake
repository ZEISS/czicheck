# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

include(FetchContent)

FetchContent_Declare(
  libCZI
  GIT_REPOSITORY https://github.com/ZEISS/libczi.git
  GIT_TAG        545098455ef50fdc7d4d8615bfa0c5bd78f52df7
)

if(NOT libCZI_POPULATED)
  message(STATUS "Fetching libCZI")
  FetchContent_Populate(libCZI)

  set(LIBCZI_BUILD_CZICMD OFF CACHE BOOL "" FORCE)
  set(LIBCZI_BUILD_DYNLIB OFF CACHE BOOL "" FORCE)
  set(LIBCZI_BUILD_UNITTESTS OFF CACHE BOOL "" FORCE)
  set(LIBCZI_DO_NOT_SET_MSVC_RUNTIME_LIBRARY  ON CACHE BOOL "" FORCE)

  add_subdirectory(${libczi_SOURCE_DIR} ${libczi_BINARY_DIR})
endif()