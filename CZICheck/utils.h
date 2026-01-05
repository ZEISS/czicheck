// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace libCZI { class IStream; }
class CCmdLineOptions;

std::uint64_t GetFileSize(const wchar_t* filename);
std::string convertToUtf8(const std::wstring& str);
std::wstring convertUtf8ToUCS2(const std::string& str);
bool icasecmp(const std::string& l, const std::string& r);
std::string trim(const std::string& str, const std::string& whitespace = " \t");
std::string GetVersionNumber();

/// Create a stream based on the command line options. If no source-stream-class is specified,
/// a standard file stream is created. Otherwise, the StreamsFactory is used with the specified
/// stream class and property bag.
///
/// \param command_line_options  The command line options containing stream class and properties.
///
/// \returns A shared pointer to the created stream.
std::shared_ptr<libCZI::IStream> CreateSourceStream(const CCmdLineOptions& command_line_options);

/// Attempts to determine the size of a stream by probing reads at various offsets.
/// This is useful for streams (like HTTP/HTTPS) that don't expose their size directly.
/// Uses binary search to efficiently find the stream's end point.
///
/// \param stream  The stream to determine the size of.
///
/// \returns The size of the stream in bytes, or 0 if the size cannot be determined (e.g., stream is larger than the largest probe offset).
std::uint64_t TryGetStreamSize(libCZI::IStream* stream);

#if CZICHECK_WIN32_ENVIRONMENT
/// A utility which is providing the command-line arguments (on Windows) as UTF8-encoded strings.
class CommandlineArgsWindowsHelper
{
private:
    std::vector<std::string> arguments_;
    std::vector<char*> pointers_to_arguments_;
public:
    /// Constructor.
    CommandlineArgsWindowsHelper();

    /// Gets an array of pointers to null-terminated, UTF8-encoded arguments. This size of this array is given
    /// by the "GetArgc"-method.
    /// Note that this pointer is only valid for the lifetime of this instance of the CommandlineArgsWindowsHelper-class.
    ///
    /// \returns    Pointer to an array of pointers to null-terminated, UTF8-encoded arguments.
    char** GetArgv();

    /// Gets the number of arguments.
    ///
    /// \returns    The number of arguments.
    int GetArgc();
};
#endif
