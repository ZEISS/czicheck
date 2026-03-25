# SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

# --------------------------------------------------------------------------
# bootstrapVcpkg.cmake – just gimme vcpkg
#
# Skip conditions:
#   1. CMAKE_TOOLCHAIN_FILE already defined (e.g. from a CMake preset)
#   2. VCPKG_ROOT env var points to a usable vcpkg installation
# If neither applies, vcpkg is cloned into <buildDir>/_deps/vcpkg and bootstrapped.
# --------------------------------------------------------------------------

# ---- 1. Toolchain already supplied (presets, -D flag) -> nothing to do -----
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "[vcpkg-bootstrap] CMAKE_TOOLCHAIN_FILE already set to '${CMAKE_TOOLCHAIN_FILE}', skipping.")
    return()
endif()

# ---- 2. Resolve vcpkg directory and executable --------------------------
if(DEFINED ENV{VCPKG_ROOT} AND NOT "$ENV{VCPKG_ROOT}" STREQUAL "")
    set(VCPKG_DIR "$ENV{VCPKG_ROOT}")
    message(STATUS "[vcpkg-bootstrap] Using VCPKG_ROOT from environment: ${VCPKG_DIR}")
else()
    set(VCPKG_DIR "${CMAKE_BINARY_DIR}/_deps/vcpkg")
endif()

if(WIN32)
    set(VCPKG_EXECUTABLE "${VCPKG_DIR}/vcpkg.exe")
else()
    set(VCPKG_EXECUTABLE "${VCPKG_DIR}/vcpkg")
endif()

# ---- 3. Clone + bootstrap only when the executable is missing -----------
if(NOT EXISTS "${VCPKG_EXECUTABLE}")
    if(NOT EXISTS "${VCPKG_DIR}/.git")
        message(STATUS "[vcpkg-bootstrap] Cloning vcpkg into ${VCPKG_DIR} ...")
        execute_process(
            COMMAND git clone --depth 1 https://github.com/microsoft/vcpkg.git "${VCPKG_DIR}"
            RESULT_VARIABLE GIT_CLONE_RESULT
        )
        if(NOT GIT_CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "[vcpkg-bootstrap] Failed to clone vcpkg from GitHub.")
        endif()
    endif()

    message(STATUS "[vcpkg-bootstrap] Bootstrapping vcpkg ...")
    if(WIN32)
        execute_process(
            COMMAND cmd /c "${VCPKG_DIR}/bootstrap-vcpkg.bat" -disableMetrics
            RESULT_VARIABLE BOOTSTRAP_RESULT
        )
    else()
        execute_process(
            COMMAND "${VCPKG_DIR}/bootstrap-vcpkg.sh" -disableMetrics
            RESULT_VARIABLE BOOTSTRAP_RESULT
        )
    endif()

    if(NOT BOOTSTRAP_RESULT EQUAL 0)
        message(FATAL_ERROR "[vcpkg-bootstrap] Failed to bootstrap vcpkg.")
    endif()
else()
    message(STATUS "[vcpkg-bootstrap] Found vcpkg at ${VCPKG_EXECUTABLE}")
endif()

# ---- 4. Point CMake at the vcpkg toolchain file -------------------------
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

set(VCPKG_FEATURE_FLAGS "manifests" CACHE STRING "")

message(STATUS "[vcpkg-bootstrap] Toolchain: ${CMAKE_TOOLCHAIN_FILE}")