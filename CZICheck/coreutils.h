// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

std::string convertToUtf8(const std::wstring& str);
std::wstring convertUtf8ToUCS2(const std::string& str);
bool icasecmp(const std::string& l, const std::string& r);
std::string trim(const std::string& str, const std::string& whitespace = " \t");
std::string GetVersionNumber();
