# SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

include(FetchContent)

FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml.git
  GIT_TAG v1.16
)

if (NOT pugixml_POPULATED)
  set(pugixml_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(pugixml_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(pugixml_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(PUGIXML_WCHAR_MODE ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(pugixml)
endif()

set(PUGIXML_INCLUDE_DIRS "${pugixml_SOURCE_DIR}/src")
message(STATUS "PugiXML include dir at " ${PUGIXML_INCLUDE_DIRS})
