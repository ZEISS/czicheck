# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT
include(FetchContent)

FetchContent_Declare(
  RapidJSON
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG        297637bc0a0f00dc504dbc1a47ebdaa577f8ae19 # master as of 2026-11-08
  PREFIX "${CMAKE_BINARY_DIR}/vendor/rapidjson"
)

if (NOT RapidJSON_POPULATED)
  set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(RapidJSON)

  set(RAPIDJSON_INCLUDE_DIRS ${rapidjson_SOURCE_DIR}/include)
endif()

message(STATUS "RapidJson include dir at " ${RAPIDJSON_INCLUDE_DIRS})
message(STATUS "RapidJson with std::string support: " ${RAPIDJSON_HAS_STDSTRING})