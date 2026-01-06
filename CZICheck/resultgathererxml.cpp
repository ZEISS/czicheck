// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererxml.h"
#include "checkerfactory.h"
#include "checks.h"
#include "cmdlineoptions.h"
#include "utils.h"

#include <sstream>
#include <string>
#include <utility>

using namespace std;

CResultGathererXml::CResultGathererXml(const CCmdLineOptions& options)
    : ResultGathererBase(options)
{
    auto decl = this->xml_document_.append_child(pugi::node_declaration);
    decl.append_attribute(kXmlVersionId) = kXmlVersionNumber;
    decl.append_attribute(kXmlEncodingId) = kXmlEncodingType;
    this->root_node_ = this->xml_document_.append_child(kTestResultsName);
    this->test_node_ = this->root_node_.append_child(kTestContainerId);
}

void CResultGathererXml::StartCheck(CZIChecks check)
{
    //this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
    this->CoreStartCheck(check);

    pugi::xml_node finding_node = this->test_node_.append_child(kTestSingleContainerId);
    string test_name { CZIChecksToString(check) };
    finding_node.append_attribute(kTestNameId) = convertUtf8ToUCS2(test_name).c_str();

    finding_node.append_child(kTestDescriptionId).text().set(convertUtf8ToUCS2(CCheckerFactory::GetCheckerDisplayName(check)).c_str());
    finding_node.append_child(kTestResultId);
    finding_node.append_child(kTestFindingContainerId);

    this->current_checker_id_ = test_name;

    //this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
}

void CResultGathererXml::FinishCheck(CZIChecks check)
{
    const IResultGatherer::CheckResult current_checker_result = this->GetCheckResultForCurrentlyActiveChecker();
    this->CoreFinishCheck(check);
    /*const auto& it = this->results_.find(check);
    const auto& result = it->second;*/
    const wstring current_checker = convertUtf8ToUCS2(this->current_checker_id_);
    for (auto current_test_node : this->test_node_.children())
    {
        auto name_attr = current_test_node.attribute(kTestNameId);
        if (name_attr && name_attr.value() == current_checker)
        {
            auto result_node = current_test_node.child(kTestResultId);
            ostringstream ss;
            if (current_checker_result.fatalMessagesCount == 0 && current_checker_result.warningMessagesCount == 0)
            {
                ss << "OK";
            }
            else if (current_checker_result.fatalMessagesCount == 0)
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

IResultGatherer::ReportFindingResult CResultGathererXml::ReportFinding(const Finding& finding)
{
   /* const auto it = this->results_.find(finding.check);
    IncrementCounter(finding.severity, it->second);*/
    this->CoreReportFinding(finding);

    const wstring current_checker = convertUtf8ToUCS2(this->current_checker_id_);
    for (auto current_test_node : this->test_node_.children())
    {
        auto name_attr = current_test_node.attribute(kTestNameId);
        if (name_attr && name_attr.value() == current_checker)
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

    return this->DetermineReportFindingResult(finding);
}

void CResultGathererXml::FinalizeChecks()
{
    ostringstream result_stream;
    switch (this->GetAggregatedResult())
    {
        case IResultGatherer::AggregatedResult::WithWarnings:
            result_stream << "WARN";
            break;
        case IResultGatherer::AggregatedResult::ErrorsDetected:
            result_stream << "FAIL";
            break;
        case IResultGatherer::AggregatedResult::OK:
            result_stream << "OK";
            break;
    }

    this->root_node_.append_child(kTestAggregatedResultId)
        .text().set(convertUtf8ToUCS2(result_stream.str()).c_str());
    auto output_version = this->root_node_.append_child(L"OutputVersion");
    output_version.append_child(L"Command")
        .text()
        .set(L"CZICheck");
    output_version.append_child(L"Version")
        .text()
        .set(convertUtf8ToUCS2(GetVersionNumber()).c_str());

    ostringstream xml_document_stream;
    this->xml_document_.save(xml_document_stream, L"  ");
    this->GetLog()->WriteStdOut(xml_document_stream.str());
}

IResultGatherer::AggregatedResult CResultGathererXml::GetAggregatedResult() const
{
    return this->CoreGetAggregatedResult();
}
