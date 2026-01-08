// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <CZICheck_Config.h>
#include "utils.h"
#include "cmdlineoptions.h"
#include "inc_libCZI.h"
#include <cwctype>
#include <memory>
#include <sstream>
#include <iostream>

#if CZICHECK_WIN32_ENVIRONMENT
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#if CZICHECK_WIN32_ENVIRONMENT
#define HAS_CODECVT
#include <Windows.h>
#endif

#if defined(HAS_CODECVT)
#include <locale>
#include <codecvt>
#include <stdlib.h>
#endif

using namespace std;

std::uint64_t GetFileSize(const wchar_t* filename)
{
#if CZICHECK_WIN32_ENVIRONMENT
    HANDLE h = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    LARGE_INTEGER li;
    GetFileSizeEx(h, &li);
    CloseHandle(h);
    return li.QuadPart;
#else
    size_t requiredSize = std::wcstombs(nullptr, filename, 0);
    std::string conv(requiredSize, 0);
    conv.resize(std::wcstombs(&conv[0], filename, requiredSize));
    struct stat sb;
    stat(conv.c_str(), &sb);
    return sb.st_size;
#endif
}

std::string convertToUtf8(const std::wstring& str)
{
#if defined(HAS_CODECVT)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::string conv = utf8_conv.to_bytes(str);
    return conv;
#else
    size_t requiredSize = std::wcstombs(nullptr, str.c_str(), 0);
    std::string conv(requiredSize, 0);
    conv.resize(std::wcstombs(&conv[0], str.c_str(), requiredSize));
    return conv;
#endif
}

std::wstring convertUtf8ToUCS2(const std::string& str)
{
#if defined(HAS_CODECVT)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8conv;
    std::wstring conv = utf8conv.from_bytes(str);
    return conv;
#else
    std::wstring conv(str.size(), 0);
    size_t size = std::mbstowcs(&conv[0], str.c_str(), str.size());
    conv.resize(size);
    return conv;
#endif
}

bool icasecmp(const std::string& l, const std::string& r)
{
    return l.size() == r.size()
        && equal(l.cbegin(), l.cend(), r.cbegin(),
            [](std::string::value_type l1, std::string::value_type r1)
            { return toupper(l1) == toupper(r1); });
}

std::string trim(const std::string& str, const std::string& whitespace /*= " \t"*/)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
    {
        return ""; // no content
    }

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string GetVersionNumber()
{
    ostringstream string_stream;
    string_stream   << CZICHECK_VERSION_MAJOR << "." \
                    << CZICHECK_VERSION_MINOR << "." \
                    << CZICHECK_VERSION_PATCH;
    return string_stream.str();
}

//-----------------------------------------------------------------------------------------

#if CZICHECK_WIN32_ENVIRONMENT
CommandlineArgsWindowsHelper::CommandlineArgsWindowsHelper()
{
    int number_arguments;
    const unique_ptr<LPWSTR, decltype(LocalFree)*> wide_argv
    {
        CommandLineToArgvW(GetCommandLineW(), &number_arguments),
            & LocalFree
    };

    this->pointers_to_arguments_.reserve(number_arguments);
    this->arguments_.reserve(number_arguments);

    for (int i = 0; i < number_arguments; ++i)
    {
        this->arguments_.emplace_back(convertToUtf8(wide_argv.get()[i]));
    }

    for (int i = 0; i < number_arguments; ++i)
    {
        this->pointers_to_arguments_.emplace_back(
            this->arguments_[i].data());
    }
}

char** CommandlineArgsWindowsHelper::GetArgv()
{
    return this->pointers_to_arguments_.data();
}

int CommandlineArgsWindowsHelper::GetArgc()
{
    return static_cast<int>(this->pointers_to_arguments_.size());
}
#endif

std::shared_ptr<libCZI::IStream> CreateSourceStream(const CCmdLineOptions& command_line_options)
{
    // If no stream class is specified, use the default file stream
    if (command_line_options.GetSourceStreamClass().empty())
    {
        return libCZI::CreateStreamFromFile(command_line_options.GetCZIFilename().c_str());
    }

    // Otherwise, use the StreamsFactory with the specified stream class and property bag
    libCZI::StreamsFactory::Initialize();
    
    libCZI::StreamsFactory::CreateStreamInfo stream_info;
    stream_info.class_name = command_line_options.GetSourceStreamClass();
   
    if (!command_line_options.GetPropertyBagForStreamClass().empty())
    {
        stream_info.property_bag = command_line_options.GetPropertyBagForStreamClass();
    }

    // For HTTP/HTTPS streams (curl), we need to convert the wstring URL to UTF-8 string
    // The curl stream class only accepts std::string URIs
    const std::string uri_utf8 = convertToUtf8(command_line_options.GetCZIFilename());
    auto source_stream = libCZI::StreamsFactory::CreateStream(stream_info, uri_utf8);

    // CreateStream does return null if the class-name is not known. If the class is valid,
    // then an exception is thrown (if something goes wrong).
    if (!source_stream)
    {
        stringstream string_stream;
        string_stream << "The input-stream-class \"" << stream_info.class_name << "\" is not valid.";
        throw std::runtime_error(string_stream.str());
    }

    return source_stream;
}
