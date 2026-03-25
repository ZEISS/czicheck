add_library(czicheck_wasm STATIC blob_stream.cpp)
add_library(czicheck::wasm ALIAS czicheck_wasm)

target_link_libraries(czicheck::wasm
        PUBLIC czicheck::wasm)

target_compile_features(czicheck::wasm PUBLIC cxx_std_20)

if (USE_WEB_PTHREADS)
    target_sources(czicheck::wasm PRIVATE proxied_blob_stream.cpp)
    target_compile_options(czicheck::wasm PUBLIC -pthread)
endif()
