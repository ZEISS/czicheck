// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "capi.h"

#include <CZICheck_Config.h>

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <algorithm>

#include "../inc_libCZI.h"
#include "../checks.h"
#include "../cmdlineoptions.h"
#include "../runchecks.h"
#include "../resultgathererjson.h"
#include "../utils.h"
#include "../consoleio.h"
#include "../checkerfactory.h"

#if CZICHECK_WIN32_ENVIRONMENT
#include <Windows.h>
#endif

#if CZICHECK_XERCESC_AVAILABLE
#include <xercesc/util/PlatformUtils.hpp>
XERCES_CPP_NAMESPACE_USE
#endif

// Dummy ILog implementation that does nothing (for internal use only)
class CNullLog : public ILog
{
public:
    void SetColor(ConsoleColor foreground, ConsoleColor background) override {}
    void WriteLineStdOut(const char* sz) override {}
    void WriteLineStdOut(const wchar_t* sz) override {}
    void WriteLineStdErr(const char* sz) override {}
    void WriteLineStdErr(const wchar_t* sz) override {}
    void WriteStdOut(const char* sz) override {}
    void WriteStdOut(const wchar_t* sz) override {}
    void WriteStdErr(const char* sz) override {}
    void WriteStdErr(const wchar_t* sz) override {}
    
    static std::shared_ptr<CNullLog> CreateInstance()
    {
        return std::make_shared<CNullLog>();
    }
};

// Helper class that mimics CCmdLineOptions but with programmatic configuration
class CValidatorOptions : public ICheckerOptions
{
private:
    std::vector<CZIChecks> checks_enabled_;
    int max_findings_;
    bool lax_parsing_;
    bool ignore_sizem_;
    std::shared_ptr<ILog> log_;
    
public:
    CValidatorOptions(const std::vector<CZIChecks>& checks, int max_findings, bool lax_parsing, bool ignore_sizem)
        : checks_enabled_(checks), max_findings_(max_findings), lax_parsing_(lax_parsing),
            ignore_sizem_(ignore_sizem), log_(CNullLog::CreateInstance())
    {
    }
    
    const std::vector<CZIChecks>& GetChecksEnabled() const { return checks_enabled_; }
    int GetMaxNumberOfMessagesToPrint() const { return max_findings_; }
    bool GetPrintDetailsOfMessages() const { return true; }  // Always true per requirements
    bool GetLaxParsingEnabled() const { return lax_parsing_; }
    bool GetIgnoreSizeMForPyramidSubBlocks() const { return ignore_sizem_; }
    const std::shared_ptr<ILog>& GetLog() const override { return log_; }
};

// Internal validator class
class CziValidator final
{
private:
    std::vector<CZIChecks> checks_;
    int max_findings_;
    bool lax_parsing_;
    bool ignore_sizem_;
    
    static std::vector<CZIChecks> BitmaskToChecks(uint64_t bitmask)
    {
        std::vector<CZIChecks> checks;
        
        if (bitmask & CZICHECK_HAS_VALID_SUBBLOCK_POSITIONS)
            checks.push_back(CZIChecks::SubBlockDirectoryPositionsWithinRange);
        if (bitmask & CZICHECK_HAS_VALID_SUBBLOCK_SEGMENTS)
            checks.push_back(CZIChecks::SubBlockDirectorySegmentValid);
        if (bitmask & CZICHECK_HAS_CONSISTENT_SUBBLOCK_DIMENSIONS)
            checks.push_back(CZIChecks::ConsistentSubBlockCoordinates);
        if (bitmask & CZICHECK_HAS_NO_DUPLICATE_SUBBLOCK_COORDINATES)
            checks.push_back(CZIChecks::DuplicateSubBlockCoordinates);
        if (bitmask & CZICHECK_DOES_NOT_USE_BINDEX)
            checks.push_back(CZIChecks::BenabledDocument);
        if (bitmask & CZICHECK_HAS_ONLY_ONE_PIXELTYPE_PER_CHANNEL)
            checks.push_back(CZIChecks::SamePixeltypePerChannel);
        if (bitmask & CZICHECK_HAS_PLANE_INDICES_STARTING_AT_ZERO)
            checks.push_back(CZIChecks::PlanesIndicesStartAtZero);
        if (bitmask & CZICHECK_HAS_CONSECUTIVE_PLANE_INDICES)
            checks.push_back(CZIChecks::PlaneIndicesAreConsecutive);
        if (bitmask & CZICHECK_ALL_SUBBLOCKS_HAVE_MINDEX)
            checks.push_back(CZIChecks::SubblocksHaveMindex);
        if (bitmask & CZICHECK_HAS_BASICALLY_VALID_METADATA)
            checks.push_back(CZIChecks::BasicMetadataValidation);
        if (bitmask & CZICHECK_HAS_XML_SCHEMA_VALID_METADATA)
        {
#if CZICHECK_XERCESC_AVAILABLE
            checks.push_back(CZIChecks::XmlMetadataSchemaValidation);
#endif
        }
        if (bitmask & CZICHECK_HAS_NO_OVERLAPPING_SCENES_AT_SCALE1)
            checks.push_back(CZIChecks::CCheckOverlappingScenesOnLayer0);
        if (bitmask & CZICHECK_HAS_VALID_SUBBLOCK_BITMAPS)
            checks.push_back(CZIChecks::CheckSubBlockBitmapValid);
#if FUTURE_CHECKS
        if (bitmask & CZICHECK_HAS_CONSISTENT_MINDICES)
            checks.push_back(CZIChecks::ConsistentMIndex);
        if (bitmask & CZICHECK_HAS_VALID_ATTACHMENT_DIR_POSITIONS)
            checks.push_back(CZIChecks::AttachmentDirectoryPositionsWithinRange);
#endif
        if (bitmask & CZICHECK_HAS_VALID_APPLIANCE_METADATA_TOPOGRAPHY)
            checks.push_back(CZIChecks::ApplianceMetadataTopographyItemValid);
            
        return checks;
    }
    
    static std::string GetCheckName(CZIChecks checkType)
    {
        switch (checkType)
        {
            case CZIChecks::SubBlockDirectoryPositionsWithinRange: 
                return "CZICHECK_HAS_VALID_SUBBLOCK_POSITIONS";
            case CZIChecks::SubBlockDirectorySegmentValid: 
                return "CZICHECK_HAS_VALID_SUBBLOCK_SEGMENTS";
            case CZIChecks::ConsistentSubBlockCoordinates: 
                return "CZICHECK_HAS_CONSISTENT_SUBBLOCK_DIMENSIONS";
            case CZIChecks::DuplicateSubBlockCoordinates: 
                return "CZICHECK_HAS_NO_DUPLICATE_SUBBLOCK_COORDINATES";
            case CZIChecks::BenabledDocument: 
                return "CZICHECK_DOES_NOT_USE_BINDEX";
            case CZIChecks::SamePixeltypePerChannel: 
                return "CZICHECK_HAS_ONLY_ONE_PIXELTYPE_PER_CHANNEL";
            case CZIChecks::PlanesIndicesStartAtZero: 
                return "CZICHECK_HAS_PLANE_INDICES_STARTING_AT_ZERO";
            case CZIChecks::PlaneIndicesAreConsecutive: 
                return "CZICHECK_HAS_CONSECUTIVE_PLANE_INDICES";
            case CZIChecks::SubblocksHaveMindex: 
                return "CZICHECK_ALL_SUBBLOCKS_HAVE_MINDEX";
            case CZIChecks::BasicMetadataValidation: 
                return "CZICHECK_HAS_BASICALLY_VALID_METADATA";
            case CZIChecks::XmlMetadataSchemaValidation: 
                return "CZICHECK_HAS_XML_SCHEMA_VALID_METADATA";
            case CZIChecks::CCheckOverlappingScenesOnLayer0: 
                return "CZICHECK_HAS_NO_OVERLAPPING_SCENES_AT_SCALE1";
            case CZIChecks::CheckSubBlockBitmapValid: 
                return "CZICHECK_HAS_VALID_SUBBLOCK_BITMAPS";
#if FUTURE_CHECKS
            case CZIChecks::ConsistentMIndex: 
                return "CZICHECK_HAS_CONSISTENT_MINDICES";
            case CZIChecks::AttachmentDirectoryPositionsWithinRange: 
                return "CZICHECK_HAS_VALID_ATTACHMENT_DIR_POSITIONS";
#endif
            case CZIChecks::ApplianceMetadataTopographyItemValid: 
                return "CZICHECK_HAS_VALID_APPLIANCE_METADATA_TOPOGRAPHY";
            default: 
                return "UNKNOWN_CHECK";
        }
    }
    
public:
    CziValidator(uint64_t checks_bitmask, int32_t max_findings, bool lax_parsing, bool ignore_sizem)
        : checks_(BitmaskToChecks(checks_bitmask)), max_findings_(max_findings),
            lax_parsing_(lax_parsing), ignore_sizem_(ignore_sizem)
    {
    }
    
    int Validate(const char* input_path, std::string& json_result, std::string& error_message)
    {
        if (input_path == nullptr)
        {
            error_message = "Input path is NULL";
            return 2;
        }
        
        try
        {
            // Create the validator options (mimics command line options)
            CValidatorOptions options(checks_, max_findings_, lax_parsing_, ignore_sizem_);
            
            // Create output stream for JSON capture
            std::ostringstream json_stream;
            
            // Open the CZI file
            std::wstring wide_path;
            try
            {
                wide_path = convertUtf8ToUCS2(input_path);
            }
            catch (const std::exception& ex)
            {
                error_message = std::string("Failed to convert path to wide string: ") + ex.what();
                return 2;
            }
            
            std::shared_ptr<libCZI::IStream> stream;
            try
            {
                stream = libCZI::CreateStreamFromFile(wide_path.c_str());
            }
            catch (const std::exception& ex)
            {
                error_message = std::string("Could not access the input file: ") + ex.what();
                return 2;
            }
            
            const auto spReader = libCZI::CreateCZIReader();
            
            try
            {
                libCZI::ICZIReader::OpenOptions open_options;
                open_options.lax_subblock_coordinate_checks = lax_parsing_;
                open_options.ignore_sizem_for_pyramid_subblocks = ignore_sizem_;
                spReader->Open(stream, &open_options);
            }
            catch (const std::exception& ex)
            {
                error_message = std::string("Could not open the CZI: ") + ex.what();
                return 2;
            }
            
            // Create result gatherer with in-memory JSON capture (minified)
            auto resultsGatherer = std::make_shared<CResultGathererJson>(options, &json_stream, true);
            
            CheckerCreateInfo checkerAdditionalInfo;
            try
            {
                checkerAdditionalInfo.totalFileSize = GetFileSize(wide_path.c_str());
            }
            catch (...)
            {
                checkerAdditionalInfo.totalFileSize = 0;  // Fallback if we can't get file size
            }
            
            // Run all checks
            std::vector<std::string> missing_checkers;
            for (auto checkType : checks_)
            {
                auto checker = CCheckerFactory::CreateChecker(checkType, spReader, *resultsGatherer, checkerAdditionalInfo);
                if (checker != nullptr)
                {
                    checker->RunCheck();
                }
                else
                {
                    // Checker could not be created - track this
                    missing_checkers.push_back(GetCheckName(checkType));
                }
            }
            
            // If some checkers couldn't be created, report as error
            if (!missing_checkers.empty())
            {
                error_message = "The following checks could not be performed (possibly not compiled in): ";
                for (size_t i = 0; i < missing_checkers.size(); ++i)
                {
                    if (i > 0) error_message += ", ";
                    error_message += missing_checkers[i];
                }
                return 4;  // Requested checks not available
            }
            
            // Finalize and get JSON result
            resultsGatherer->FinalizeChecks();
            json_result = json_stream.str();
            
            return 0;  // Success
        }
        catch (const std::exception& ex)
        {
            error_message = std::string("Validation failed with exception: ") + ex.what();
            return 2;
        }
        catch (...)
        {
            error_message = "Validation failed with unknown exception";
            return 2;
        }
    }
};

// C API Implementation

extern "C" CAPI_EXPORT void* CreateValidator(uint64_t checks_bitmask, int32_t max_findings, bool lax_parsing, bool ignore_sizem)
{
    // Validate parameters
    if (checks_bitmask == 0)
    {
        return nullptr;  // No checks enabled
    }
    
    try
    {
#if CZICHECK_WIN32_ENVIRONMENT
        CoInitialize(NULL);
#endif

#if CZICHECK_XERCESC_AVAILABLE
        static bool xerces_initialized = false;
        if (!xerces_initialized)
        {
            XMLPlatformUtils::Initialize();
            xerces_initialized = true;
        }
#endif
        
        return new CziValidator(checks_bitmask, max_findings, lax_parsing, ignore_sizem);
    }
    catch (...)
    {
        return nullptr;
    }
}

extern "C" CAPI_EXPORT int ValidateFile(void* validator, const char* input_path, 
                                        char* json_buffer, uint64_t* json_buffer_size,
                                        char* error_message, uint64_t* error_message_length)
{
    if (validator == nullptr)
    {
        return 3;  // Invalid validator pointer
    }
    
    if (input_path == nullptr || json_buffer_size == nullptr)
    {
        return 3;  // Invalid parameters
    }
    
    CziValidator* val = static_cast<CziValidator*>(validator);
    
    std::string json_result;
    std::string error_msg;
    
    int result = val->Validate(input_path, json_result, error_msg);
    
    if (result != 0)
    {
        // Validation failed - copy error message if buffer provided
        if (error_message != nullptr && error_message_length != nullptr && *error_message_length > 0)
        {
            uint64_t copy_len = error_msg.length() < (*error_message_length - 1) ? error_msg.length() : (*error_message_length - 1);
            std::memcpy(error_message, error_msg.c_str(), copy_len);
            error_message[copy_len] = '\0';
            *error_message_length = copy_len;
        }
        return result;
    }
    
    // Check if buffer is large enough
    uint64_t required_size = json_result.length() + 1;  // +1 for null terminator
    
    if (json_buffer == nullptr || *json_buffer_size < required_size)
    {
        *json_buffer_size = required_size;
        return 1;  // Buffer too small
    }
    
    // Copy JSON result to buffer
    std::memcpy(json_buffer, json_result.c_str(), json_result.length());
    json_buffer[json_result.length()] = '\0';
    *json_buffer_size = required_size;
    
    return 0;  // Success
}

extern "C" CAPI_EXPORT void DestroyValidator(void* validator)
{
    if (validator != nullptr)
    {
        CziValidator* val = static_cast<CziValidator*>(validator);
        delete val;
        
#if CZICHECK_WIN32_ENVIRONMENT
        CoUninitialize();
#endif
    }
}

extern "C" CAPI_EXPORT void GetLibVersion(int32_t* major, int32_t* minor, int32_t* patch)
{
    if (major != nullptr)
    {
        *major = std::atoi(CZICHECK_VERSION_MAJOR);
    }
    if (minor != nullptr)
    {
        *minor = std::atoi(CZICHECK_VERSION_MINOR);
    }
    if (patch != nullptr)
    {
        *patch = std::atoi(CZICHECK_VERSION_PATCH);
    }
}

extern "C" CAPI_EXPORT bool GetLibVersionString(char* buffer, uint64_t* size)
{
    if (size == nullptr)
    {
        return false;
    }
    
    std::string version_str = std::string(CZICHECK_VERSION_MAJOR) + "." +
                            std::string(CZICHECK_VERSION_MINOR) + "." +
                            std::string(CZICHECK_VERSION_PATCH);
    
    uint64_t required_size = version_str.length() + 1;  // +1 for null terminator
    
    if (buffer == nullptr || *size < required_size)
    {
        *size = required_size;
        return false;
    }
    
    std::memcpy(buffer, version_str.c_str(), version_str.length());
    buffer[version_str.length()] = '\0';
    *size = required_size;
    
    return true;
}
