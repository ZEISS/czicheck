# SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "CMAKE_TOOLCHAIN_FILE already set to '${CMAKE_TOOLCHAIN_FILE}', skipping vcpkg bootstrap.")
    return()
endif()

if(DEFINED ENV{VCPKG_ROOT} AND NOT "$ENV{VCPKG_ROOT}" STREQUAL "")
    set(VCPKG_DIR "$ENV{VCPKG_ROOT}")
else()
    set(VCPKG_DIR "${CMAKE_BINARY_DIR}/_deps/vcpkg")
endif()

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
    if(NOT EXISTS "${VCPKG_DIR}/.git")
        message(STATUS "vcpkg not found")
        
        # We clone the latest vcpkg (master) instead of checking out the specific builtin-baseline commit.
        # Motivation:
        # 1. vcpkg is designed to handle versioning via the vcpkg.json manifest file. The 'builtin-baseline'
        #    field in vcpkg.json tells vcpkg which versions of ports to use, regardless of the vcpkg tool version.
        # 2. Using the latest vcpkg tool ensures we have the newest features, bug fixes, and performance improvements.
        # 3. Checking out the vcpkg repo to the baseline commit is considered an anti-pattern because it downgrades
        #    the tool itself, potentially missing critical fixes or binary caching capabilities.
        message(STATUS "Cloning vcpkg...")
        execute_process(
            COMMAND git clone https://github.com/microsoft/vcpkg.git "${VCPKG_DIR}"
            RESULT_VARIABLE GIT_CLONE_RESULT
        )
        if(NOT GIT_CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone vcpkg from GitHub.")
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