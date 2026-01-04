// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <CZICheck_Config.h>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include "checks.h"
#include "consoleio.h"

class CCmdLineOptions
{
public:
    /// Defines the output encoding formats supported by the application.
    /// This enum specifies the different formats in which check results can be encoded
    /// and presented to the user or integrated into automated workflows.
    enum class OutputEncodingFormat
    {
        /// Plain text format - human-readable output with minimal formatting.
        /// Optimized for console display and simple log files.
        TEXT,
        
        /// JSON (JavaScript Object Notation) format - structured data format.
        /// Suitable for programmatic consumption, integration with web services,
        /// and automated processing pipelines.
        JSON,
        
        /// XML (Extensible Markup Language) format - hierarchical data format.
        /// Useful for enterprise integration scenarios and systems requiring
        /// schema validation.
        XML,
    };

    /// Defines different modes for handling fatal errors during checking operations.
    /// This enum controls the fail-fast behavior when fatal errors are encountered
    /// during the execution of CZI file checks.
    enum class FailFastMode
    {
        /// No fail-fast behavior. All checks continue to run even when fatal errors are encountered.
        Disabled,
        
        /// Stop execution immediately when a fatal error is encountered within a specific checker.
        /// Each checker handles its own fatal errors independently.
        FailFastForFatalErrorsPerChecker,
        
        /// Stop the entire checking operation immediately when any fatal error is encountered
        /// across all checkers. This provides the fastest termination on fatal errors.
        FailFastForFatalErrorsOverall,
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
    std::string source_stream_class_;
    std::map<std::string, std::string> property_bag_;
    FailFastMode fail_fast_mode_{ FailFastMode::Disabled };
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
    [[nodiscard]] const std::shared_ptr<ILog>& GetLog() const { return this->log_; }
    [[nodiscard]] const OutputEncodingFormat GetOutputEncodingFormat() const { return this->result_encoding_type_; }
    [[nodiscard]] const std::string& GetSourceStreamClass() const { return this->source_stream_class_; }
    [[nodiscard]] const std::map<std::string, std::string>& GetPropertyBag() const { return this->property_bag_; }
    [[nodiscard]] FailFastMode GetFailFastMode() const { return this->fail_fast_mode_; }
private:
    static bool ParseBooleanArgument(const std::string& argument_key, const std::string& argument_value, bool* boolean_value, std::string* error_message);
    static bool ParseChecksArgument(const std::string& str, std::vector<CZIChecks>* checks_enabled, std::string* error_message);
    static bool ParseEncodingArgument(const std::string& str, OutputEncodingFormat& encoding, std::string& error_message);
    static bool ParseFailFastArgument(const std::string& str, FailFastMode& fail_fast_mode, std::string& error_message);

    /// Information about a "checker item" and whether it is to be added or removed.
    struct CheckerToRunInfo
    {
        std::string checkerName;             ///< The "short name" identifying the checker.
        bool addOrRemoveChecker{ false };    ///< Whether this checker is to be added (true) or removed (false).
    };

    static bool TryParseCheckerAddOrRemove(const std::string& str, CheckerToRunInfo* info);

    static std::string GetCheckerListHelpText();
    static std::string GetAppDescription();
    void PrintVersionInfo();
};
