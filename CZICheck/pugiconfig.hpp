
#ifndef HEADER_PUGICONFIG_HPP
#define HEADER_PUGICONFIG_HPP

// Uncomment this to enable wchar_t mode
#define PUGIXML_WCHAR_MODE

// Uncomment this to enable compact mode
// #define PUGIXML_COMPACT

// Uncomment this to disable XPath
// #define PUGIXML_NO_XPATH

// Uncomment this to disable STL
// #define PUGIXML_NO_STL

// Uncomment this to disable exceptions
// #define PUGIXML_NO_EXCEPTIONS

// Set this to control attributes for public classes/functions, i.e.:
// #define PUGIXML_API __declspec(dllexport) // to export all public symbols from DLL
// #define PUGIXML_CLASS __declspec(dllimport) // to import all classes from DLL
// #define PUGIXML_FUNCTION __fastcall // to set calling conventions to all public functions to fastcall
// In absence of PUGIXML_CLASS/PUGIXML_FUNCTION definitions PUGIXML_API is used instead

// Tune these constants to adjust memory-related behavior
// #define PUGIXML_MEMORY_PAGE_SIZE 32768
// #define PUGIXML_MEMORY_OUTPUT_STACK 10240
// #define PUGIXML_MEMORY_XPATH_PAGE_SIZE 4096

// Tune this constant to adjust max nesting for XPath queries
// #define PUGIXML_XPATH_DEPTH_LIMIT 1024

// Uncomment this to switch to header-only version
// #define PUGIXML_HEADER_ONLY

// Uncomment this to enable long long support
#define PUGIXML_HAS_LONG_LONG

#endif