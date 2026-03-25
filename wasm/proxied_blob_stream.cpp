#include "proxied_blob_stream.h"
#include "blob_stream.h"

#include <emscripten/proxying.h>
#include <emscripten/threading.h>

#include <cstring>
#include <mutex>

namespace wasm {
namespace {

class ProxiedBlobStream final : public IStream {
public:
    ProxiedBlobStream(int js_handle, uint64_t file_size)
        : inner_(make_blob_stream(js_handle, file_size))
        , size_(file_size)
        , js_handle_(js_handle)
        , queue_(em_proxying_queue_create())
    {}

    ~ProxiedBlobStream() override {
        em_proxying_queue_destroy(queue_);
    }

    size_t read(uint64_t offset, void* buf, size_t count) const override {
        // If we're already on the main thread, read directly — proxying
        // to ourselves would deadlock.
        if (emscripten_is_main_runtime_thread()) {
            return inner_->read(offset, buf, count);
        }

        // Serialize all proxied reads globally.  EM_ASYNC_JS (used by the
        // inner BlobStream) suspends via asyncify.  Emscripten asyncify only
        // supports one active unwind per thread — if two proxied callbacks
        // run on the main thread concurrently, their asyncify states corrupt
        // each other, causing garbled CZI reads and crashes.
        static std::mutex s_proxy_mu;
        std::lock_guard lock(s_proxy_mu);

        // Stage parameters for the proxied callback.
        proxied_offset_ = offset;
        proxied_buf_ = buf;
        proxied_count_ = count;
        proxied_result_ = 0;

        // Proxy the read to the main browser thread where EM_ASYNC_JS works.
        emscripten_proxy_sync(
            queue_,
            emscripten_main_runtime_thread_id(),
            [](void* ctx) {
                auto& self = *static_cast<ProxiedBlobStream*>(ctx);
                self.proxied_result_ = self.inner_->read(
                    self.proxied_offset_, self.proxied_buf_, self.proxied_count_);
            },
            const_cast<ProxiedBlobStream*>(this));

        return proxied_result_;
    }

    uint64_t Size() const override { return size_; }

    std::unique_ptr<IStream> Clone() const override {
        return std::make_unique<ProxiedBlobStream>(js_handle_, size_);
    }

private:
    std::unique_ptr<IStream> inner_;
    uint64_t size_;
    int js_handle_;
    em_proxying_queue* queue_;

    // Mutable scratch used during proxied calls (single-threaded access
    // per call, serialized by emscripten_proxy_sync).
    mutable uint64_t proxied_offset_{};
    mutable void* proxied_buf_{};
    mutable size_t proxied_count_{};
    mutable size_t proxied_result_{};
};

} // anon

std::unique_ptr<IStream> make_proxied_blob_stream(int js_handle, uint64_t file_size) {
    return std::make_unique<ProxiedBlobStream>(js_handle, file_size);
}

}
