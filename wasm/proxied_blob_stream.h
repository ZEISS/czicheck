#pragma once
#include "stream.h"
#include <memory>

namespace wasm {

/// Create a read-only stream that proxies I/O calls to the main thread.
/// Required when reading a JS File/Blob from a worker thread, because
/// EM_ASYNC_JS (used by BlobStream) can only run on the main thread.
///
/// @param js_handle  Index into Module._fileHandles[] on the JS side.
/// @param file_size  Size of the file in bytes.
std::unique_ptr<IStream> make_proxied_blob_stream(int js_handle, uint64_t file_size);

}
