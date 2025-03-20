// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <CZICheck_Config.h>
#include <regex>
#include <unordered_set>
#include <memory>
#include <utility>
#include <algorithm>
#include <string>
#include "cmdlineoptions.h"
#include "utils.h"
#include "checkerfactory.h"

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

using namespace std;

CCmdLineOptions::CCmdLineOptions(std::shared_ptr<ILog> log)
    : log_(std::move(log)),
    max_number_of_findings_to_print_(3),
    print_details_of_messages_(false),
    lax_parsing_enabled_(false),
    ignore_sizem_for_pyramid_subblocks_(false)
{
    // as default, all the checkers which are not flagged "isOptIn" are enabled
    CCheckerFactory::EnumerateCheckers(
        [&](const CCheckerFactory::CheckersInfo& checkerInfo)->bool
        {
            if (!checkerInfo.isOptIn)
            {
                this->checks_enabled_.emplace_back(checkerInfo.checkerType);
            }

            return true;
        });
}

CCmdLineOptions::ParseResult CCmdLineOptions::Parse(int argc, char** argv)
{
    CLI::App app{ CCmdLineOptions::GetAppDescription(), "CZICheck" };

    ostringstream string_stream;
    string_stream <<
        "The exit code of CZICheck is" << endl <<
        " 0  - all checks completed without an error or a warning" << endl <<
        " 1  - the checks found some warnings, but no errors" << endl <<
        " 2  - the checks gave one or more errors" << endl <<
        " 5  - the CZI-file could not be read or opened" << endl <<
        " 10 - the command line arguments are invalid" << endl << endl;
    string_stream << CCmdLineOptions::GetCheckerListHelpText();
    app.footer(string_stream.str());

    // CLI11-validator for the option "--checks".
    struct ChecksValidator : public CLI::Validator
    {
        ChecksValidator()
        {
            this->func_ = [](const std::string& str) -> string
                {
                    string error_message;
                    const bool parsed_ok = CCmdLineOptions::ParseChecksArgument(str, nullptr, &error_message);
                    if (!parsed_ok)
                    {
                        throw CLI::ValidationError(error_message);
                    }

                    return {};
                };
        }
    };

    struct EncodingValidator : public CLI::Validator
    {
        EncodingValidator()
        {
            this->func_ = [](const std::string& str) -> string
            {
                string error_message;
                OutputEncodingFormat encoding_format;
                const bool parsed_ok = CCmdLineOptions::ParseEncodingArgument(str, encoding_format, error_message);
                if (!parsed_ok)
                {
                    throw CLI::ValidationError(error_message);
                }

                return {};
            };
        }
    };

    struct BooleanArgumentValidator : public CLI::Validator
    {
        explicit BooleanArgumentValidator(const std::string& argument_key)
        {
            this->func_ = [argument_key](const std::string& str) -> string
                {
                    string error_message;
                    const bool parsed_ok = CCmdLineOptions::ParseBooleanArgument(argument_key, str, nullptr, &error_message);
                    if (!parsed_ok)
                    {
                        throw CLI::ValidationError(error_message);
                    }

                    return {};
                };
        }
    };

    // CLI11-validator for the option "--printdetails".
    struct PrintDetailsValidator : public BooleanArgumentValidator
    {
        PrintDetailsValidator() : BooleanArgumentValidator("printdetails")
        {}
    };

    // CLI11-validator for the option "--laxparsing".
    struct LaxParsingValidator : public BooleanArgumentValidator
    {
        LaxParsingValidator() : BooleanArgumentValidator("laxparsing")
        {}
    };

    static const ChecksValidator checks_validator;
    static const EncodingValidator encodings_validator;
    static const PrintDetailsValidator print_details_validator;
    static const LaxParsingValidator lax_parsing_validator;

    string source_filename_options;
    string checks_enable_options;
    int max_number_of_findings_option;
    string print_details_option;
    string lax_parsing_enabled;
    string ignore_sizem_for_pyramid_subblocks_enabled;
    string result_encoding_option;
    app.add_option("-s,--source", source_filename_options, "Specify the CZI-file to be checked.")
        ->option_text("FILENAME")
        ->required();
    app.add_option("-c,--checks", checks_enable_options,
        "Specifies a comma-separated list of short-names of checkers\n"
        "to run. In addition to the short-names, the following\n"
        "\"set-names\" are possible : 'default' and 'all'. 'default'\n"
        "means \"all checkers which are not flagged as opt-in\", and\n"
        "'all' means \"all available checkers\". A minus ('-')\n"
        "prepended to the checker-short-name (or set-name) means that\n"
        "this checker or set is to be removed from the list of\n"
        "checkers to run.\n"
        "A plus('+') means that it is to be added, and this is also\n"
        "the default if no plus or minus is prepended.\n"
        "Examples:\n"
        "\"default, -benabled\" : run all checkers in the \"default set\"\n"
        "                       without the checker 'benabled'\n"
        "\"+benabled, +planesstartindex\" : run only the checkers\n"
        "                                 'benabled' and\n"
        "                                 'planesstartindex'\n"
        "Default is 'default'.\n")
        ->option_text("CHECKS-TO-BE-RUN")
        ->check(checks_validator);
    app.add_option("-m,--maxfindings", max_number_of_findings_option,
        "Specifies how many findings are to be reported and printed\n"
        "(for every check).\n"
        "A negative number means 'no limit'. Default is 3.\n")
        ->option_text("INTEGER")
        ->default_val(3);
    app.add_option("-d,--printdetails", print_details_option,
        "Specifies whether to print details (if available) with a\n"
        "finding. The argument may be one of 'true', 'false', 'yes'\n"
        "or 'no'.\n")
        ->option_text("BOOLEAN")
        ->check(print_details_validator);
    app.add_option("-l,--laxparsing", lax_parsing_enabled,
        "Specifies whether lax parsing for file opening is enabled.\n"
        "This option allows operation on some malformed CZIs which would\n"
        "otherwise not be analyzable at all.\n"
        "The argument may be one of 'true', 'false', 'yes'\n"
        "or 'no'. Default is 'no'.\n")
        ->option_text("BOOLEAN")
        ->check(lax_parsing_validator);
    app.add_option("-i, --ignoresizem", ignore_sizem_for_pyramid_subblocks_enabled,
        "Specifies whether to ignore the 'SizeM' field for pyramid subblocks.\n"
        "This option allows operation on some malformed CZIs which would\n"
        "otherwise not be analyzable at all.\n"
        "The argument may be one of 'true', 'false', 'yes'\n"
        "or 'no'. Default is 'false'.\n")
        ->option_text("BOOLEAN");
    app.add_option("-e,--encoding", result_encoding_option,
        "Specifies which encoding should be used for result reporting.\n"
        "The argument may be one of 'json', 'xml', 'text'. Default is 'text'.\n")
        ->option_text("ENCODING")
        ->check(encodings_validator);

    // Parse the command line arguments
    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::CallForHelp& e)
    {
        app.exit(e);
        return ParseResult::Exit;
    }
    catch (const CLI::ParseError& e)
    {
        app.exit(e);
        return ParseResult::Error;
    }

    if (source_filename_options.empty())
    {
        this->log_->WriteLineStdErr("No CZI-file specified.");
        return ParseResult::Error;
    }

    this->czi_filename_ = convertUtf8ToUCS2(source_filename_options);
    this->max_number_of_findings_to_print_ = (max_number_of_findings_option >= 0) ? max_number_of_findings_option : -1;
    if (!checks_enable_options.empty())
    {
        string error_message;
        const bool parsed_ok = CCmdLineOptions::ParseChecksArgument(checks_enable_options, &this->checks_enabled_, &error_message);
        if (!parsed_ok)
        {
            this->log_->WriteLineStdErr(error_message);
            return ParseResult::Error;
        }
    }

    if (!print_details_option.empty())
    {
        string error_message;
        const bool parsed_ok = CCmdLineOptions::ParseBooleanArgument("printdetails", print_details_option, &this->print_details_of_messages_, &error_message);
        if (!parsed_ok)
        {
            this->log_->WriteLineStdErr(error_message);
            return ParseResult::Error;
        }
    }

    if (!lax_parsing_enabled.empty())
    {
        string error_message;
        const bool parsed_ok = CCmdLineOptions::ParseBooleanArgument("laxparsing", lax_parsing_enabled, &this->lax_parsing_enabled_, &error_message);
        if (!parsed_ok)
        {
            this->log_->WriteLineStdErr(error_message);
            return ParseResult::Error;
        }
    }

    if (!ignore_sizem_for_pyramid_subblocks_enabled.empty())
    {
        string error_message;
        const bool parsed_ok = CCmdLineOptions::ParseBooleanArgument("ignoresizem", ignore_sizem_for_pyramid_subblocks_enabled, &this->ignore_sizem_for_pyramid_subblocks_, &error_message);
        if (!parsed_ok)
        {
            this->log_->WriteLineStdErr(error_message);
            return ParseResult::Error;
        }
    }

    if (!result_encoding_option.empty())
    {
        string error_message;
        const bool parsed_ok = CCmdLineOptions::ParseEncodingArgument(result_encoding_option, this->result_encoding_type_, error_message);
        if (!parsed_ok)
        {
            this->log_->WriteLineStdErr(error_message);
            return ParseResult::Error;
        }
    }

    return  ParseResult::OK;
}

/*static*/std::string CCmdLineOptions::GetCheckerListHelpText()
{
    ostringstream string_stream;
    string_stream << "Available checkers (checkers enabled with the default set are marked with '*'):" << endl;
    CCheckerFactory::EnumerateCheckers(
        [&](const CCheckerFactory::CheckersInfo& checkerInfo)->bool
        {
            if (checkerInfo.isOptIn)
            {
                string_stream << "  ";
            }
            else
            {
                string_stream << "* ";
            }

            string_stream << "\"" << checkerInfo.shortName << "\" -> " << checkerInfo.displayName;

            string_stream << endl;
            return true;
        });

    return string_stream.str();
}

/*static*/std::string CCmdLineOptions::GetAppDescription()
{
    int libCZI_version_major, libCZI_version_minor, libCZI_version_patch;
    libCZI::GetLibCZIVersion(&libCZI_version_major, &libCZI_version_minor, &libCZI_version_patch);
    ostringstream string_stream;
    string_stream << "CZICheck version " << CZICHECK_VERSION_MAJOR << '.' << CZICHECK_VERSION_MINOR << '.' << CZICHECK_VERSION_PATCH;
    string_stream << ", using libCZI version " << libCZI_version_major << '.' << libCZI_version_minor << '.' << libCZI_version_patch << endl;
    return string_stream.str();
}

/*static*/bool CCmdLineOptions::ParseEncodingArgument(const std::string& str, OutputEncodingFormat& encoding, std::string& error_message)
{
    error_message.clear();

    if (icasecmp("text", str))
    {
        encoding = OutputEncodingFormat::TEXT;
        return true;
    }

    if (icasecmp("json", str))
    {
        encoding = OutputEncodingFormat::JSON;
        return true;
    }

    if (icasecmp("xml", str))
    {
        encoding = OutputEncodingFormat::XML;
        return true;
    }

    error_message = "The output encoding option you passed is unknown.";
    return false;
}

/*static*/bool CCmdLineOptions::ParseChecksArgument(const std::string& str, std::vector<CZIChecks>* checks_enabled, std::string* error_message)
{
    if (error_message != nullptr)
    {
        error_message->clear();
    }

    const regex re(R"([\s|,;]+)");	// tokenize at space, comma, semicolon or pipe
    const sregex_token_iterator it{ str.begin(), str.end(), re , -1 };
    vector<string> tokenized_items{ it, {} };
    tokenized_items.erase(
        std::remove_if(tokenized_items.begin(), tokenized_items.end(),
        [](const string& s) -> bool
        {
            return s.empty();
        }),
        tokenized_items.end());

    if (tokenized_items.empty())
    {
        if (error_message != nullptr)
        {
            *error_message = "No checkers specified";
        }

        return false;
    }

    // necessary for clang 11 it seems, c.f. https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
    struct EnumCIZChecksClassHash
    {
        size_t operator()(CZIChecks t) const
        {
            return static_cast<size_t>(t);
        }
    };

    unordered_set<CZIChecks, EnumCIZChecksClassHash> checks_to_run;
    for (const auto& tokenized : tokenized_items)
    {
        // parse for a leading "+" or "-" (which means we should include or exclude the specified checker)
        CheckerToRunInfo checkerToRunInfo;
        if (!CCmdLineOptions::TryParseCheckerAddOrRemove(tokenized, &checkerToRunInfo))
        {
            if (error_message != nullptr)
            {
                stringstream ss;
                ss << "Invalid checker encountered \"" << tokenized << "\"\n";
                *error_message = ss.str();
            }

            return false;
        }

        // check for the "default" short-name to select all checkers that are not flagged as opt-in
        if (icasecmp("default", checkerToRunInfo.checkerName))
        {
            if (checkerToRunInfo.addOrRemoveChecker)
            {
                // we are to add all checkers which are "not opt-in"
                CCheckerFactory::EnumerateCheckers([&](const CCheckerFactory::CheckersInfo& checkerInfo)->bool
                    {
                        if (!checkerInfo.isOptIn)
                        {
                            checks_to_run.insert(checkerInfo.checkerType);
                        }

                        return true;
                    });
            }
            else
            {
                // we are to remove all checkers which are "not opt-in"
                CCheckerFactory::EnumerateCheckers([&](const CCheckerFactory::CheckersInfo& checkerInfo)->bool
                    {
                        if (!checkerInfo.isOptIn)
                        {
                            checks_to_run.erase(checkerInfo.checkerType);
                        }

                        return true;
                    });
            }
        }
        else if (icasecmp("all", checkerToRunInfo.checkerName))
        {
            if (checkerToRunInfo.addOrRemoveChecker)
            {
                // we have to add all checkers (irrespective of whether they "opt-in" or not)
                CCheckerFactory::EnumerateCheckers([&](const CCheckerFactory::CheckersInfo& checkerInfo)->bool
                    {
                        checks_to_run.insert(checkerInfo.checkerType);
                        return true;
                    });
            }
            else
            {
                // '-all' is rather pointless, but well, just remove all we have
                checks_to_run.clear();
            }
        }
        else
        {
            // try to parse the short-name (we could consider to ignore unknown short-names here)
            CZIChecks checkType;
            if (!CCheckerFactory::TryParseShortName(checkerToRunInfo.checkerName, checkType))
            {
                if (error_message != nullptr)
                {
                    stringstream ss;
                    ss << "Invalid checker encountered \"" << tokenized << "\"\n";
                    *error_message = ss.str();
                }

                return false;
            }

            if (checkerToRunInfo.addOrRemoveChecker)
            {
                checks_to_run.insert(checkType);
            }
            else
            {
                checks_to_run.erase(checkType);
            }
        }
    }

    if (checks_enabled != nullptr)
    {
        checks_enabled->clear();
        checks_enabled->reserve(checks_to_run.size());
        copy(checks_to_run.begin(), checks_to_run.end(), back_inserter(*checks_enabled));

        // Sort the vector by the numerical value of the enum items
        std::sort(checks_enabled->begin(), checks_enabled->end());
    }

    return true;
}

/*static*/bool CCmdLineOptions::TryParseCheckerAddOrRemove(const std::string& str, CheckerToRunInfo* info)
{
    const regex regex(R"(\s*([\+|-]?)\s*(\S+)\s*)");
    std::cmatch matches;
    std::regex_match(str.c_str(), matches, regex);
    if (matches.size() != 3)
    {
        return false;
    }

    bool add_or_remove = true;
    if (matches.str(1) == "-")
    {
        // if we have a "-" before the short-name, then this means "remove this checker"
        add_or_remove = false;
    }

    if (info != nullptr)
    {
        info->addOrRemoveChecker = add_or_remove;
        info->checkerName = matches.str(2);
    }

    return true;
}

/*static*/bool CCmdLineOptions::ParseBooleanArgument(const std::string& argument_key, const std::string& argument_value, bool* boolean_value, string* error_message)
{
    const auto trimmed = trim(argument_value);
    if (icasecmp(trimmed, "yes") || icasecmp(trimmed, "true") || icasecmp(trimmed, "1"))
    {
        if (boolean_value != nullptr)
        {
            *boolean_value = true;
        }

        return true;
    }
    else if (icasecmp(trimmed, "no") || icasecmp(trimmed, "false") || icasecmp(trimmed, "0"))
    {
        if (boolean_value != nullptr)
        {
            *boolean_value = false;
        }

        return true;
    }

    if (error_message != nullptr)
    {
        ostringstream string_stream;
        string_stream << "Invalid argument for option '" << argument_key << "': \"" << trimmed << "\"";
        *error_message = string_stream.str();
    }

    return false;
}
