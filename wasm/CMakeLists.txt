# SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

add_library(czicheck_wasm_io STATIC blob_stream.cpp)

target_include_directories(czicheck_wasm_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(czicheck_wasm_io PUBLIC cxx_std_17)

if (USE_WEB_PTHREADS)
    target_sources(czicheck_wasm_io PRIVATE proxied_blob_stream.cpp)
    target_compile_options(czicheck_wasm_io PUBLIC -pthread)
    target_compile_definitions(czicheck_wasm_io PUBLIC USE_WEB_PTHREADS)
endif()

add_executable(czicheck_wasm
    wasm_main.cpp
    wasm_runchecks.cpp
)

set_target_properties(czicheck_wasm PROPERTIES
    CXX_STANDARD 17
    OUTPUT_NAME "czicheck"
    SUFFIX ".js"
)

target_link_libraries(czicheck_wasm PRIVATE
    czicheck_wasm_io
    czicheck_core
)

set(EM_LINK_FLAGS
    "--bind"
    # Asyncify is for blob reads
    "-sASYNCIFY"
    "-sASYNCIFY_STACK_SIZE=65536" # I doubt we need this but just in case
    "-sALLOW_MEMORY_GROWTH=1"
    "--shell-file" "${CMAKE_SOURCE_DIR}/assets/shell.html"
    # Export ccall/cwrap for optional direct C calls from JS
    "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
)

if (USE_WEB_PTHREADS)
    list(APPEND EM_LINK_FLAGS
        "-pthread"
        "-sPTHREAD_POOL_SIZE=4"
    )
endif()

target_link_options(czicheck_wasm PRIVATE ${EM_LINK_FLAGS})

if (USE_WEB_PTHREADS)
    target_compile_options(czicheck_wasm PRIVATE -pthread)
    target_compile_definitions(czicheck_wasm PRIVATE USE_WEB_PTHREADS)
endif()

# Copy coi-serviceworker alongside the output
add_custom_command(TARGET czicheck_wasm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/coi-serviceworker.min.js"
        "$<TARGET_FILE_DIR:czicheck_wasm>/coi-serviceworker.min.js"
    COMMENT "Copying coi-serviceworker.min.js to output directory"
)

find_program(EMRUN emrun REQUIRED)
add_custom_target(serve
    COMMAND ${EMRUN} --no_emrun_detect $<TARGET_FILE:czicheck_wasm>
    DEPENDS czicheck_wasm
    COMMENT "Serving CZICheck in browser..."
)
