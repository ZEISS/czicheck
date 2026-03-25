#pragma once
#include "stream.h"
#include <libCZI.h>

namespace wasm {

class CziStreamAdapter final : public libCZI::IStream {
public:
    explicit CziStreamAdapter(const wasm::IStream& stream) : stream_(stream) {}

    void Read(std::uint64_t offset, void* pv, std::uint64_t size, std::uint64_t* ptrBytesRead) override {
        size_t n = stream_.read(offset, pv, static_cast<size_t>(size));
        if (ptrBytesRead)
            *ptrBytesRead = n;
    }

private:
    const wasm::IStream& stream_;
};

}
