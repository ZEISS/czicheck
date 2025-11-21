# SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "CMAKE_TOOLCHAIN_FILE already set to '${CMAKE_TOOLCHAIN_FILE}', skipping vcpkg bootstrap.")
    return()
endif()

set(BUILDING_IN_VISUAL_STUDIO OFF)
if(DEFINED CMAKE_GENERATOR_INSTANCE OR DEFINED CMAKE_VS_INSTANCE)
    set(BUILDING_IN_VISUAL_STUDIO ON)
endif()

set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/external/vcpkg")
if(WIN32)
    set(VCPKG_EXECUTABLE "${VCPKG_DIR}/vcpkg.exe")
else()
    set(VCPKG_EXECUTABLE "${VCPKG_DIR}/vcpkg")
endif()

if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/vcpkg" AND NOT DEFINED VCPKG_EXECUTABLE)
    set(VCPKG_DIR "$ENV{VCPKG_ROOT}")
    if(WIN32)
        set(VCPKG_EXECUTABLE "${VCPKG_DIR}/vcpkg.exe")
    else()
        set(VCPKG_EXECUTABLE "${VCPKG_DIR}/vcpkg")
    endif()
    message(STATUS "Using pre-existing vcpkg at ${VCPKG_DIR}")
endif()

if(NOT EXISTS "${VCPKG_EXECUTABLE}")
    # if(BUILDING_IN_VISUAL_STUDIO AND DEFINED ENV{VCPKG_ROOT})
    #     message(STATUS "Visual Studio detected with VCPKG_ROOT, skipping local bootstrap.")
    #     return()
    # endif()

    if(NOT EXISTS "${VCPKG_DIR}/.git")
        message(STATUS "vcpkg not found")
        message(STATUS "Attempting to find baseline from vcpkg.json...")
        file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" _vcpkg_manifest)
        string(REGEX MATCH "\"builtin-baseline\"[ \t]*:[ \t]*\"([0-9a-f]+)\"" _ ${_vcpkg_manifest})
        set(VCPKG_BASELINE_COMMIT "${CMAKE_MATCH_1}")

        if(NOT VCPKG_BASELINE_COMMIT OR NOT VCPKG_BASELINE_COMMIT MATCHES "^[0-9a-f]+$")
            message(WARNING
                "Failed to extract builtin-baseline from vcpkg.json. "
                "Ensure that your manifest defines a valid baseline like:\n"
                "  \"builtin-baseline\": \"<commit sha>\""
            )
        else()
            message(STATUS "Determined baseline commit: ${VCPKG_BASELINE_COMMIT}")
        endif()

        message(STATUS "Cloning vcpkg...")
        execute_process(
            COMMAND git clone --depth 1 https://github.com/microsoft/vcpkg.git "${VCPKG_DIR}"
            RESULT_VARIABLE GIT_CLONE_RESULT
        )
        if(NOT GIT_CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone vcpkg from GitHub.")
        endif()
        
        if(DEFINED VCPKG_BASELINE_COMMIT)
          message(STATUS "Fetching vcpkg baseline commit: ${VCPKG_BASELINE_COMMIT}")
          execute_process(
            COMMAND ${CMAKE_COMMAND} -E chdir "${VCPKG_DIR}" git fetch --depth 1 origin ${VCPKG_BASELINE_COMMIT}
            RESULT_VARIABLE GIT_FETCH_RESULT
            )
          if(NOT GIT_FETCH_RESULT EQUAL 0)
            message(FATAL_ERROR
              "Failed to fetch vcpkg baseline commit ${VCPKG_BASELINE_COMMIT}. "
              "Check that your builtin-baseline is correct and exists in the vcpkg repo."
            )
          endif()
        endif()
    else()
        message(STATUS "Found vcpkg directory at ${VCPKG_DIR}, skipping clone.")
    endif()

    message(STATUS "Bootstrapping vcpkg...")
    if(WIN32)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E chdir "${VCPKG_DIR}" cmd /c bootstrap-vcpkg.bat
            RESULT_VARIABLE BOOTSTRAP_RESULT
        )
    else()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E chdir "${VCPKG_DIR}" ./bootstrap-vcpkg.sh
            RESULT_VARIABLE BOOTSTRAP_RESULT
        )
    endif()

    if(NOT BOOTSTRAP_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to bootstrap vcpkg.")
    endif()
else()
    message(STATUS "Found vcpkg executable at ${VCPKG_EXECUTABLE}, skipping bootstrap.")
endif()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

set(VCPKG_FEATURE_FLAGS "manifests" CACHE STRING "")

message(STATUS "vcpkg is ready. Toolchain file: ${CMAKE_TOOLCHAIN_FILE}")