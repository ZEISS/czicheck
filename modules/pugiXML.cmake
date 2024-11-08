# SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

include(FetchContent)

FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml.git
  GIT_TAG v1.14
)

if (NOT pugixml_POPULATED)
  set(pugixml_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(pugixml_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(pugixml_BUILD_TESTS OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(pugixml)

  set(PUGIXML_INCLUDE_DIRS ${pugixml_SOURCE_DIR}/include)
endif()

message(STATUS "PugiXML include dir at " ${PUGIXML_INCLUDE_DIRS})
