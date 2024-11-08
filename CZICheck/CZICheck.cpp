// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <CZICheck_Config.h>
#include "consoleio.h"
#include "cmdlineoptions.h"
#include "runchecks.h"
#include "utils.h"
#include "inc_libCZI.h"

#if CZICHECK_UNIX_ENVIRONMENT
#include <clocale>
#endif

#if CZICHECK_WIN32_ENVIRONMENT
#include <Windows.h>
#endif

#if CZICHECK_XERCESC_AVAILABLE
#include <xercesc/util/PlatformUtils.hpp>
XERCES_CPP_NAMESPACE_USE
#endif

int main(int argc, char** argv)
{
#if CZICHECK_WIN32_ENVIRONMENT
    CoInitialize(NULL);
#endif
#if CZICHECK_UNIX_ENVIRONMENT
    setlocale(LC_CTYPE, "");
#endif

    const auto log = CConsoleLog::CreateInstance();

    CCmdLineOptions options(log);
    CCmdLineOptions::ParseResult arguments_parse_result = CCmdLineOptions::ParseResult::Exit;

#if CZICHECK_WIN32_ENVIRONMENT
    {
        CommandlineArgsWindowsHelper args_helper;
        arguments_parse_result = options.Parse(args_helper.GetArgc(), args_helper.GetArgv());
    }
#endif
#if CZICHECK_UNIX_ENVIRONMENT
    arguments_parse_result = options.Parse(argc, argv);
#endif

#if CZICHECK_XERCESC_AVAILABLE
    XMLPlatformUtils::Initialize();
#endif

    int return_code;
    if (arguments_parse_result == CCmdLineOptions::ParseResult::OK)
    {
        CRunChecks runChecks(options, log);
        IResultGatherer::AggregatedResult result;
        if (runChecks.Run(result))
        {
            switch (result)
            {
                case IResultGatherer::AggregatedResult::OK:
                    // log->WriteStdOut("\n\nResult: OK\n");
                    return_code = 0;
                    break;
                case IResultGatherer::AggregatedResult::WithWarnings:
                    // log->WriteStdOut("\n\nResult: With Warnings\n");
                    return_code = 1;
                    break;
                case IResultGatherer::AggregatedResult::ErrorsDetected:
                    // log->WriteStdOut("\n\nResult: Errors Detected\n");
                    return_code = 2;
                    break;
                default:
                    // log->WriteStdOut("\n\nResult: something unexpected happened\n");
                    return_code = 3;
                    break;
            }
        }
        else
        {
            return_code = 5;
        }
    }
    else
    {
        return_code = 10;
    }

#if CZICHECK_XERCESC_AVAILABLE
    XMLPlatformUtils::Terminate();
#endif
#if CZICHECK_WIN32_ENVIRONMENT
    CoUninitialize();
#endif

    return return_code;
}
