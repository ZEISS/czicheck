#pragma once
#include "stream.h"
#include <memory>

namespace wasm {

/// Create a read-only stream backed by a JS File/Blob object.
/// @param js_handle Index into Module._fileHandles[] on the JS side.
/// @param file_size Size of the file in bytes (queried from JS).
std::unique_ptr<IStream> make_blob_stream(int js_handle, uint64_t file_size);

}
