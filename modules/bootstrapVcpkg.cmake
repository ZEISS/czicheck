if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)

    set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/external/vcpkg")

    if(WIN32)
        set(VCPKG_EXECUTABLE "${VCPKG_DIR}/vcpkg.exe")
    else()
        set(VCPKG_EXECUTABLE "${VCPKG_DIR}/vcpkg")
    endif()

    if(EXISTS "${VCPKG_EXECUTABLE}")
        message(STATUS "Found existing vcpkg executable at ${VCPKG_EXECUTABLE}")
        set(VCPKG_IS_CLONED TRUE)
        set(VCPKG_IS_BOOTSTRAPPED TRUE)
    else()
        set(VCPKG_IS_CLONED FALSE)
        if(EXISTS "${VCPKG_DIR}/.git" OR EXISTS "${VCPKG_DIR}/README.md")
            set(VCPKG_IS_CLONED TRUE)
        endif()

        if(VCPKG_IS_CLONED)
            message(STATUS "Found vcpkg folder at ${VCPKG_DIR}")
        else()
            message(STATUS "vcpkg not found - cloning into ${VCPKG_DIR}...")
            execute_process(
                COMMAND git clone --depth 1 https://github.com/microsoft/vcpkg.git "${VCPKG_DIR}"
                RESULT_VARIABLE GIT_CLONE_RESULT
            )
            if(NOT GIT_CLONE_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to clone vcpkg from GitHub.")
            endif()
            set(VCPKG_IS_CLONED TRUE)
        endif()

        if(NOT EXISTS "${VCPKG_EXECUTABLE}")
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
            message(STATUS "vcpkg bootstrapped successfully.")
        else()
            message(STATUS "vcpkg already bootstrapped.")
        endif()
    endif()

    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")

    set(VCPKG_FEATURE_FLAGS "manifests" CACHE STRING "")

    message(STATUS "vcpkg is ready. Toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

endif()
