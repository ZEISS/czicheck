# Pre-populating cache so CMake default set has these values
# This is for libczi because for wasm target we don't propagate the vcpkg ccflags properly.
set(CMAKE_C_FLAGS   "-pthread" CACHE STRING "C flags for Emscripten threads")
set(CMAKE_CXX_FLAGS "-pthread" CACHE STRING "CXX flags for Emscripten threads")