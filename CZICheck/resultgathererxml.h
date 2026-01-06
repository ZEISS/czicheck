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
    CheckResult GetAggregatedCounts() const override;

private:
    static const wchar_t* kXmlVersionId;
    static const wchar_t* kXmlVersionNumber;
    static const wchar_t* kXmlEncodingId;
    static const wchar_t* kXmlEncodingType;
    static const wchar_t* kTestResultsName;
    static const wchar_t* kTestContainerId;
    static const wchar_t* kTestSingleContainerId;
    static const wchar_t* kTestNameId;
    static const wchar_t* kTestDescriptionId;
    static const wchar_t* kTestResultId;
    static const wchar_t* kTestAggregatedResultId;
    static const wchar_t* kTestFindingContainerId;
    static const wchar_t* kTestFindingId;
    static const wchar_t* kTestSeverityId;
    static const wchar_t* kTestDetailsId;
};

