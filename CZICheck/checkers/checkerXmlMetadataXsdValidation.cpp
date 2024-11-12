// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerXmlMetadataXsdValidation.h"

#if CZICHECK_XERCESC_AVAILABLE

#include <exception>
#include <sstream>
#include <memory>
#include <string>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/validators/common/Grammar.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include "checkerXmlMetadataXsdSchema.h"

using namespace libCZI;
using namespace std;

XERCES_CPP_NAMESPACE_USE

/*static*/const char* CCheckXmlMetadataXsdValidation::kDisplayName = "validate the XML-metadata against XSD-schema";
/*static*/const char* CCheckXmlMetadataXsdValidation::kShortName = "xmlmetadataschema";

class ParserErrorHandler : public ErrorHandler
{
private:
    IResultGatherer& result_gatherer_;

    void reportParseError(const SAXParseException& ex)
    {
        IResultGatherer::Finding finding(CCheckXmlMetadataXsdValidation::kCheckType);
        finding.severity = IResultGatherer::Severity::Fatal;
        const std::unique_ptr<char, void(*)(char*)> upMsg(XMLString::transcode(ex.getMessage()), [](char* p)->void {XMLString::release(&p); });
        ostringstream ss;
        ss << "(" << ex.getLineNumber() << "," << ex.getColumnNumber() << "): " << upMsg.get();
        finding.information = ss.str();
        this->result_gatherer_.ReportFinding(finding);
    }

    void reportParseWarning(const SAXParseException& ex)
    {
        IResultGatherer::Finding finding(CCheckXmlMetadataXsdValidation::kCheckType);
        finding.severity = IResultGatherer::Severity::Warning;
        const std::unique_ptr<char, void(*)(char*)> upMsg(XMLString::transcode(ex.getMessage()), [](char* p)->void {XMLString::release(&p); });
        ostringstream ss;
        ss << "(" << ex.getLineNumber() << "," << ex.getColumnNumber() << ") : " << upMsg.get();
        finding.information = ss.str();
        this->result_gatherer_.ReportFinding(finding);
    }
public:
    ParserErrorHandler() = delete;

    explicit ParserErrorHandler(IResultGatherer& result_gatherer)
        : result_gatherer_(result_gatherer)
    {
    }

    void warning(const SAXParseException& ex) override
    {
        this->reportParseError(ex);
    }

    void error(const SAXParseException& ex) override
    {
        this->reportParseError(ex);
    }

    void fatalError(const SAXParseException& ex) override
    {
        this->reportParseWarning(ex);
    }

    void resetErrors() override
    {
    }
};

CCheckXmlMetadataXsdValidation::CCheckXmlMetadataXsdValidation(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckXmlMetadataXsdValidation::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckXmlMetadataXsdValidation::kCheckType);

    const string xml = this->GetCziMetadataXml();

    if (!xml.empty())
    {
        XercesDOMParser dom_parser;
        ParserErrorHandler parser_error_handler(this->result_gatherer_);

        dom_parser.setErrorHandler(&parser_error_handler);

        size_t size_zen_complete_xsd;
        const char* zen_complete_xsd = GetZenCompleteXsd(&size_zen_complete_xsd);

        const MemBufInputSource xml_metadata_schema(reinterpret_cast<const XMLByte*>(zen_complete_xsd), size_zen_complete_xsd, "schema.xsd", false);

        // note: the grammar object is owned by the parser (so, we must not delete it)
        Grammar* g = dom_parser.loadGrammar(xml_metadata_schema, Grammar::SchemaGrammarType, true);

        dom_parser.setValidationScheme(XercesDOMParser::Val_Always);
        dom_parser.setDoNamespaces(true);
        dom_parser.useCachedGrammarInParse(true);
        dom_parser.setDoSchema(true);
        dom_parser.setValidationConstraintFatal(true);
        dom_parser.setExitOnFirstFatalError(false);
        dom_parser.setExternalNoNamespaceSchemaLocation("");
        dom_parser.setDisableDefaultEntityResolution(true); // Disable DTD processing in order to prevent XXE attacks (c.f. https://owasp.org/www-community/vulnerabilities/XML_External_Entity_(XXE)_Processing).

        const MemBufInputSource czi_xml_metadata(reinterpret_cast<const XMLByte*>(xml.c_str()), xml.size(), "dummy", false);
        dom_parser.parse(czi_xml_metadata);
    }

    this->result_gatherer_.FinishCheck(CCheckXmlMetadataXsdValidation::kCheckType);
}

std::string CCheckXmlMetadataXsdValidation::GetCziMetadataXml()
{
    const auto czi_metadata = this->GetCziMetadataAndReportErrors(CCheckXmlMetadataXsdValidation::kCheckType);
    if (czi_metadata)
    {
        return czi_metadata->GetXml();
    }

    return {};
}

#endif
