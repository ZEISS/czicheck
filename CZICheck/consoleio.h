// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <CZICheck_Config.h>
#include <string>
#include <memory>
#include <cstdint>

#if CZICHECK_WIN32_ENVIRONMENT
#include <Windows.h>
#endif

/// Values that represent console colors.
enum class ConsoleColor : unsigned char
{
    BLACK = 0,
    DARK_BLUE,
    DARK_GREEN,
    DARK_CYAN,
    DARK_RED,
    DARK_MAGENTA,
    DARK_YELLOW,
    DARK_WHITE,
    LIGHT_BLACK,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    LIGHT_YELLOW,
    WHITE,
    DEFAULT
};

/// This interface is used to write to the console. It is intended to provide platform independent
/// ability to change the color of the text on the console.
class ILog
{
public:
    virtual void SetColor(ConsoleColor foreground, ConsoleColor background) = 0;

    virtual void WriteLineStdOut(const char* sz) = 0;
    virtual void WriteLineStdOut(const wchar_t* sz) = 0;
    virtual void WriteLineStdErr(const char* sz) = 0;
    virtual void WriteLineStdErr(const wchar_t* sz) = 0;

    virtual void WriteStdOut(const char* sz) = 0;
    virtual void WriteStdOut(const wchar_t* sz) = 0;
    virtual void WriteStdErr(const char* sz) = 0;
    virtual void WriteStdErr(const wchar_t* sz) = 0;

    void WriteLineStdOut(const std::string& str)
    {
        this->WriteLineStdOut(str.c_str());
    }

    void WriteLineStdOut(const std::wstring& str)
    {
        this->WriteLineStdOut(str.c_str());
    }

    void WriteLineStdErr(const std::string& str)
    {
        this->WriteLineStdErr(str.c_str());
    }

    void WriteLineStdErr(const std::wstring& str)
    {
        this->WriteLineStdErr(str.c_str());
    }

    void WriteStdOut(const std::string& str)
    {
        this->WriteStdOut(str.c_str());
    }

    void WriteStdOut(const std::wstring& str)
    {
        this->WriteStdOut(str.c_str());
    }

    void WriteStdErr(const std::string& str)
    {
        this->WriteStdErr(str.c_str());
    }

    void WriteStdErr(const std::wstring& str)
    {
        this->WriteStdErr(str.c_str());
    }

    virtual ~ILog() = default;
};

/// Implementation of the ILog interface that writes to the console. This implementation is intended
/// to provide platform independent ability to change the color of the text on the console. This includes
/// older Windows version that do not support ANSI escape sequences (aka Virtual Terminal Sequences, 
/// c.f. https://learn.microsoft.com/en-us/windows/console/classic-vs-vt).
/// If stdout/stderr is redirected to a file, the color information is ignored.
class CConsoleLog : public ILog
{
private:
#if CZICHECK_WIN32_ENVIRONMENT
    HANDLE consoleHandle;
    std::uint16_t defaultConsoleColor;
#endif
#if CZICHECK_UNIX_ENVIRONMENT
    bool isTerminalOutput;
#endif
public:
    static std::shared_ptr<ILog> CreateInstance();

    CConsoleLog();

    void SetColor(ConsoleColor foreground, ConsoleColor background) override;

    void WriteLineStdOut(const char* sz) override;
    void WriteLineStdOut(const wchar_t* sz) override;
    void WriteLineStdErr(const char* sz) override;
    void WriteLineStdErr(const wchar_t* sz) override;

    void WriteStdOut(const char* sz) override;
    void WriteStdOut(const wchar_t* sz) override;
    void WriteStdErr(const char* sz) override;
    void WriteStdErr(const wchar_t* sz) override;
private:
#if CZICHECK_WIN32_ENVIRONMENT
    std::uint16_t GetColorAttribute(ConsoleColor foreground, ConsoleColor background);
#endif
#if CZICHECK_UNIX_ENVIRONMENT
    void SetTextColorAnsi(ConsoleColor foreground, ConsoleColor background);
#endif
};
