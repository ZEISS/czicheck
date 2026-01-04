// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IResultGatherer.h"
#include "resultgathererbase.h"
#include "cmdlineoptions.h"
#include "checks.h"

#include "pugixml.hpp"
#include <string>

class CResultGathererXml : public IResultGatherer, ResultGathererBase
{
private:
    std::string current_checker_id_;

    pugi::xml_document xml_document_;
    pugi::xml_node root_node_;
    pugi::xml_node test_node_;

public:
    explicit CResultGathererXml(const CCmdLineOptions& options);
    void StartCheck(CZIChecks check) override;
    ReportFindingResult ReportFinding(const Finding& finding) override;
    void FinishCheck(CZIChecks check) override;
    void FinalizeChecks() override;

private:
    static constexpr const wchar_t* kXmlVersionId = L"version";
    static constexpr const wchar_t* kXmlVersionNumber = L"1.0";
    static constexpr const wchar_t* kXmlEncodingId = L"encoding";
    static constexpr const wchar_t* kXmlEncodingType = L"utf-8";
    static constexpr const wchar_t* kTestResultsName = L"TestResults";
    static constexpr const wchar_t* kTestContainerId = L"Tests";
    static constexpr const wchar_t* kTestSingleContainerId = L"Test";
    static constexpr const wchar_t* kTestNameId = L"Name";
    static constexpr const wchar_t* kTestDescriptionId = L"Description";
    static constexpr const wchar_t* kTestResultId = L"Result";
    static constexpr const wchar_t* kTestAggregatedResultId = L"AggregatedResult";
    static constexpr const wchar_t* kTestFindingContainerId = L"Findings";
    static constexpr const wchar_t* kTestFindingId = L"Finding";
    static constexpr const wchar_t* kTestSeverityId = L"Severity";
    static constexpr const wchar_t* kTestDetailsId = L"Details";
};

