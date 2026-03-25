// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <CZICheck_Config.h>
#include "coreutils.h"
#include <cwctype>
#include <sstream>

#if CZICHECK_WIN32_ENVIRONMENT
#define HAS_CODECVT
#include <Windows.h>
#endif

#if defined(HAS_CODECVT)
#include <locale>
#include <codecvt>
#include <cstdlib>
#endif

using namespace std;

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
        return "";
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
