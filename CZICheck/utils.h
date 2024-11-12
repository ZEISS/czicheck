// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::uint64_t GetFileSize(const wchar_t* filename);
std::string convertToUtf8(const std::wstring& str);
std::wstring convertUtf8ToUCS2(const std::string& str);
bool icasecmp(const std::string& l, const std::string& r);
std::string trim(const std::string& str, const std::string& whitespace = " \t");
std::string GetVersionNumber();

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
