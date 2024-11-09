// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererxml.h"
#include "checkerfactory.h"
#include "checks.h"
#include "cmdlineoptions.h"
#include "utils.h"

#include <codecvt>
#include <iostream>
#ifdef unix
#include <locale>
#endif
#include <string>
#include <utility>

using namespace std;

CResultGathererXml::CResultGathererXml(const CCmdLineOptions& options)
    : options_(options)
{
    auto decl = this->xml_document_.append_child(pugi::node_declaration);
    decl.append_attribute(kXmlVersionId) = kXmlVersionNumber;
    decl.append_attribute(kXmlEncodingId) = kXmlEncodingType;
    this->root_node_ = this->xml_document_.append_child(kTestResultsName);
    this->test_node_ = this->root_node_.append_child(kTestContainerId);
}

void CResultGathererXml::StartCheck(CZIChecks check)
{
    const auto checker_display_name = CCheckerFactory::GetCheckerDisplayName(check).c_str();
    this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
    pugi::xml_node finding_node = this->test_node_.append_child(kTestSingleContainerId);
    string test_name { CZIChecksToString(check) };
    finding_node.append_attribute(kTestNameId) = convertUtf8ToUCS2(test_name).c_str();

    finding_node.append_child(kTestDescriptionId).text().set(convertUtf8ToUCS2(CCheckerFactory::GetCheckerDisplayName(check)).c_str());
    finding_node.append_child(kTestResultId);
    finding_node.append_child(kTestFindingContainerId);

    this->current_checker_id_ = test_name;

    this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
}

void CResultGathererXml::FinishCheck(CZIChecks check)
{
    const auto& it = this->results_.find(check);
    const auto& result = it->second;
    wstring_convert<codecvt_utf8<wchar_t>> utf8_conv;
    for (auto current_test_node : this->test_node_.children())
    {
        auto name_attr = current_test_node.attribute(kTestNameId);
        if (name_attr && utf8_conv.to_bytes(name_attr.value()) == this->current_checker_id_)
        {
            auto result_node = current_test_node.child(kTestResultId);
            ostringstream ss;
            if (result.fatalMessagesCount == 0 && result.warningMessagesCount == 0)
            {
                ss << "OK";
            }
            else if (result.fatalMessagesCount == 0)
            {
                ss << "WARN";
            }
            else
            {
                ss << "FAIL";
            }

            result_node.text().set(convertUtf8ToUCS2(ss.str()).c_str());
        }
    }
}

void CResultGathererXml::ReportFinding(const Finding& finding)
{
    const auto it = this->results_.find(finding.check);
    const auto no_of_findings_so_far = it->second.GetTotalMessagesCount();
    IncrementCounter(finding.severity, it->second);

    wstring_convert<codecvt_utf8<wchar_t>> utf8_conv;
    for (auto current_test_node : this->test_node_.children())
    {
        auto name_attr = current_test_node.attribute(kTestNameId);
        if (name_attr && utf8_conv.to_bytes(name_attr.value()) == this->current_checker_id_)
        {
            auto findings_container = current_test_node.child(kTestFindingContainerId);

            auto current_finding = findings_container.append_child(kTestFindingId);
            current_finding.append_child(kTestSeverityId)
                .text()
                .set(convertUtf8ToUCS2(finding.FindingSeverityToString()).c_str());
            current_finding.append_child(kTestDescriptionId)
                .text()
                .set(convertUtf8ToUCS2(finding.information).c_str());
            current_finding.append_child(kTestDetailsId)
                .text()
                .set(convertUtf8ToUCS2(finding.details).c_str());

            break;
        }
    }
}

void CResultGathererXml::FinalizeChecks()
{
    ostringstream ss;
    switch (this->GetAggregatedResult()) {
        case IResultGatherer::AggregatedResult::WithWarnings:
            ss << "WARN";
            break;
        case IResultGatherer::AggregatedResult::ErrorsDetected:
            ss << "FAIL";
            break;
        case IResultGatherer::AggregatedResult::OK:
            ss << "OK";
            break;
    }

    this->root_node_.append_child(L"AggregatedResult")
        .text().set(convertUtf8ToUCS2(ss.str()).c_str());
    this->xml_document_.save(std::cout, L" ");
}

