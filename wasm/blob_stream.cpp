#include "blob_stream.h"
#include <emscripten.h>
#include <cstdint>

// Async JS function: reads bytes from a JS File handle into WASM memory.
// Uses Blob.slice().arrayBuffer() — requires -sASYNCIFY.
EM_ASYNC_JS(int, js_blob_read, (int handle, double offset, void* buf, int count), {
    const file = Module._fileHandles[handle];
    if (!file) {
        console.error('[js_blob_read] Invalid file handle', handle);
        return 0;
    }
    const start = offset;
    const end = Math.min(start + count, file.size);
    if (start >= file.size) {
        console.warn(`[js_blob_read] Offset >= file.size: offset=${start}, file.size=${file.size}`);
        return 0;
    }
    if (count > 10485760) { // ~10MB
        console.warn(`[js_blob_read] Large count requested: count=${count}, offset=${start}, end=${end}, file.size=${file.size}`);
    }
    if (start > 4294967295 || end > 4294967295) {
        console.warn(`[js_blob_read] Large offset: start=${start}, end=${end}, file.size=${file.size}`);
    }
    try {
        const blob = file.slice(start, end);
        const ab = await blob.arrayBuffer();
        const bytes = new Uint8Array(ab);
        HEAPU8.set(bytes, buf);
        return bytes.length;
    } catch (e) {
        console.error(`[js_blob_read] Exception:`, e, `offset=${start}, end=${end}, count=${count}, file.size=${file.size}`);
        return 0;
    }
});

namespace wasm {
namespace {

class BlobStream final : public IStream {
public:
    BlobStream(int handle, uint64_t size) : handle_(handle), size_(size) {}

    size_t read(uint64_t offset, void* buf, size_t count) const override {
        if (offset >= size_) return 0;
        int request = static_cast<int>(std::min<uint64_t>(count, size_ - offset));
        int n = js_blob_read(handle_, static_cast<double>(offset), buf, request);
        return static_cast<size_t>(n);
    }

    uint64_t Size() const override { return size_; }

    std::unique_ptr<IStream> Clone() const override {
        return std::make_unique<BlobStream>(handle_, size_);
    }

private:
    int handle_;
    uint64_t size_;
};

}

std::unique_ptr<IStream> make_blob_stream(int js_handle, uint64_t file_size) {
    return std::make_unique<BlobStream>(js_handle, file_size);
}

}
