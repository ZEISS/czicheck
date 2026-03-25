// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <CZICheck_Config.h>
#include "utils.h"
#include "cmdlineoptions.h"
#include "inc_libCZI.h"
#include <memory>
#include <sstream>

#if CZICHECK_WIN32_ENVIRONMENT
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
        std::stringstream string_stream;
        string_stream << "The input-stream-class \"" << stream_info.class_name << "\" is not valid.";
        throw std::runtime_error(string_stream.str());
    }

    return source_stream;
}
