// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <CZICheck_Config.h>
#include <vector>
#include <memory>
#include <string>
#include "checks.h"
#include "consoleio.h"

// Abstract interface for checker options
class ICheckerOptions
{
public:
    virtual ~ICheckerOptions() = default;
    virtual const std::shared_ptr<ILog>& GetLog() const = 0;
};

class CCmdLineOptions : public ICheckerOptions
{
public:
    enum class OutputEncodingFormat
    {
        TEXT,
        JSON,
        XML
    };
private:
    std::shared_ptr<ILog> log_;
    std::wstring czi_filename_;
    std::vector<CZIChecks> checks_enabled_;
    int max_number_of_findings_to_print_;
    bool print_details_of_messages_;
    bool lax_parsing_enabled_;
    bool ignore_sizem_for_pyramid_subblocks_;
    OutputEncodingFormat result_encoding_type_ { OutputEncodingFormat::TEXT };
public:
    /// Values that represent the result of the "Parse"-operation.
    enum class ParseResult
    {
        OK,     ///< An enum constant representing the result "arguments successfully parsed, operation can start".
        Exit,   ///< An enum constant representing the result "operation complete, the program should now be terminated, e.g. the synopsis was printed".
        Error   ///< An enum constant representing the result "the was an error parsing the command line arguments, program should terminate".
    };

    explicit CCmdLineOptions(std::shared_ptr<ILog> log);

    /// Parses the command line arguments. 
    ///
    /// \param          argc    The number of strings passed with 'argv'.
    /// \param [in]     argv    The array of arguments (expected as UTF8-encoded strings).
    ///
    /// \returns    An enum indicating the result of the operation.
    ParseResult Parse(int argc, char** argv);

    [[nodiscard]] const std::wstring& GetCZIFilename() const { return this->czi_filename_; }
    [[nodiscard]] int GetMaxNumberOfMessagesToPrint() const { return this->max_number_of_findings_to_print_; }
    [[nodiscard]] bool GetPrintDetailsOfMessages() const { return this->print_details_of_messages_; }
    [[nodiscard]] bool GetLaxParsingEnabled() const { return this->lax_parsing_enabled_; }
    [[nodiscard]] bool GetIgnoreSizeMForPyramidSubBlocks() const { return this->ignore_sizem_for_pyramid_subblocks_; }
    [[nodiscard]] const std::vector<CZIChecks>& GetChecksEnabled() const { return this->checks_enabled_; }
    [[nodiscard]] const std::shared_ptr<ILog>& GetLog() const override { return this->log_; }
    [[nodiscard]] const OutputEncodingFormat GetOutputEncodingFormat() const { return this->result_encoding_type_; }
private:
    static bool ParseBooleanArgument(const std::string& argument_key, const std::string& argument_value, bool* boolean_value, std::string* error_message);
    static bool ParseChecksArgument(const std::string& str, std::vector<CZIChecks>* checks_enabled, std::string* error_message);
    static bool ParseEncodingArgument(const std::string& str, OutputEncodingFormat& encoding, std::string& error_message);

    /// Information about a "checker item" and whether it is to be added or removed.
    struct CheckerToRunInfo
    {
        std::string checkerName;             ///< The "short name" identifying the checker.
        bool addOrRemoveChecker{ false };    ///< Whether this checker is to be added (true) or removed (false).
    };

    static bool TryParseCheckerAddOrRemove(const std::string& str, CheckerToRunInfo* info);

    static std::string GetCheckerListHelpText();
    static std::string GetAppDescription();
};
