#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>

namespace wasm {

/// Read-only random-access stream.
class IStream {
public:
    virtual ~IStream() = default;

    /// Read up to count bytes starting at offset into buf,
    /// @return Number of bytes actually read (may be less at end of stream).
    virtual size_t read(uint64_t offset, void* buf, size_t count) const = 0;

    /// @return Total size of the stream in bytes.
    virtual uint64_t Size() const = 0;

    /// Create an independent copy of this stream reading the same underlying data.
    /// Each clone owns its own file handle / seek state and can be used concurrently. #TODO Fact-check concurrency claim....
    /// Returns nullptr if cloning is not supported.
    virtual std::unique_ptr<IStream> Clone() const { return nullptr; }
};

}
