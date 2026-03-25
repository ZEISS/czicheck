// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "consoleio.h"
#include <string>
#include <sstream>
#include <memory>

namespace wasm {

/// ILog implementation that captures all output into a string buffer.
/// Collect checker output and return it to JS.
class StringLog : public ILog
{
public:
    void SetColor(ConsoleColor /*foreground*/, ConsoleColor /*background*/) override {}

    void WriteLineStdOut(const char* sz) override { out_ << sz << '\n'; }
    void WriteLineStdOut(const wchar_t* sz) override { WriteWide(out_, sz); out_ << '\n'; }
    void WriteLineStdErr(const char* sz) override { err_ << sz << '\n'; }
    void WriteLineStdErr(const wchar_t* sz) override { WriteWide(err_, sz); err_ << '\n'; }

    void WriteStdOut(const char* sz) override { out_ << sz; }
    void WriteStdOut(const wchar_t* sz) override { WriteWide(out_, sz); }
    void WriteStdErr(const char* sz) override { err_ << sz; }
    void WriteStdErr(const wchar_t* sz) override { WriteWide(err_, sz); }

    std::string GetStdOut() const { return out_.str(); }

    std::string GetStdErr() const { return err_.str(); }

    void Clear() { out_.str({}); err_.str({}); }

    static std::shared_ptr<ILog> CreateInstance()
    {
        return std::make_shared<StringLog>();
    }

private:
    std::ostringstream out_;
    std::ostringstream err_;

    // TODO: Remove this placeholder with proper wstring handling....
    static void WriteWide(std::ostringstream& stream, const wchar_t* sz)
    {
        for (; *sz; ++sz)
        {
            stream << static_cast<char>(*sz <= 127 ? *sz : '?');
        }
    }
};

} // namespace wasm
