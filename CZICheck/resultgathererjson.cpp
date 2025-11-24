// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererjson.h"
#include "checkerfactory.h"
#include "utils.h"

#include <ostream>
#include <string>
#include <utility>

using namespace std;

CResultGathererJson::CResultGathererJson(const ICheckerOptions& options, std::ostringstream* output_stream, bool minified)
    : options_(options), output_stream_(output_stream), minified_(minified)
{
    this->json_document_.SetArray();
    this->test_results_ = rapidjson::Value(rapidjson::kArrayType);
}

void CResultGathererJson::StartCheck(CZIChecks check)
{
    const auto checker_display_name = CCheckerFactory::GetCheckerDisplayName(check).c_str();

    auto allocator = this->json_document_.GetAllocator();

    rapidjson::Value test_run(rapidjson::kObjectType);
    test_run.AddMember(rapidjson::Value(kTestNameId, allocator), rapidjson::Value().SetString(CZIChecksToString(check), allocator), allocator);
    test_run.AddMember(rapidjson::Value(kTestDescriptionId, allocator), rapidjson::Value().SetString(checker_display_name, allocator), allocator);
    test_run.AddMember(rapidjson::Value(kTestResultId, allocator), rapidjson::Value(rapidjson::kStringType), allocator);
    test_run.AddMember(rapidjson::Value(kTestFindingsId, allocator), rapidjson::Value(rapidjson::kArrayType), allocator);

    this->test_results_.PushBack(test_run, allocator);
    this->current_checker_id = std::string(CZIChecksToString(check));
    this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
}

void CResultGathererJson::FinishCheck(CZIChecks check)
{
    const auto& it = this->results_.find(check);
    const auto& result = it->second;

    auto allocator = this->json_document_.GetAllocator();
    for (int res { 0 }; res < this->test_results_.Size(); ++res)
    {
        if (this->test_results_[res][kTestNameId].GetString() == this->current_checker_id)
        {
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

            this->test_results_[res][kTestResultId].SetString(ss.str().c_str(), allocator);
        }
    }
}

void CResultGathererJson::ReportFinding(const Finding& finding)
{
    const auto it = this->results_.find(finding.check);
    const auto no_of_findings_so_far = it->second.GetTotalMessagesCount();
    IncrementCounter(finding.severity, it->second);

    auto allocator = this->json_document_.GetAllocator();
    for (int res { 0 }; res < this->test_results_.Size(); ++res)
    {
        if (this->test_results_[res][kTestNameId].GetString() == this->current_checker_id)
        {
            rapidjson::Value current_finding(rapidjson::kObjectType);
            current_finding.SetObject()
                .AddMember(rapidjson::Value(kTestSeverityId, allocator), rapidjson::Value().SetString(finding.FindingSeverityToString(), allocator), allocator)
                .AddMember(rapidjson::Value(kTestDescriptionId, allocator), rapidjson::Value().SetString(finding.information.c_str(), allocator), allocator)
                .AddMember(rapidjson::Value(kTestDetailsId, allocator), rapidjson::Value().SetString(finding.details.c_str(), allocator), allocator);
            this->test_results_[res][kTestFindingsId].PushBack(current_finding, allocator);
        }
    }
}

void CResultGathererJson::FinalizeChecks()
{
    this->json_document_.SetObject();
    auto allocator = this->json_document_.GetAllocator();

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

    this->json_document_.AddMember(rapidjson::Value(kTestAggregationId, allocator), rapidjson::Value().SetString(ss.str().c_str(), allocator), allocator);
    this->json_document_.AddMember(rapidjson::Value(kTestContainerId, allocator), this->test_results_, allocator);

    rapidjson::Value output_version(rapidjson::kObjectType);
    output_version.AddMember(rapidjson::Value("command", allocator), rapidjson::Value("CZICheck", allocator), allocator);
    output_version.AddMember(rapidjson::Value("version", allocator), rapidjson::Value(GetVersionNumber().c_str(), allocator), allocator);

    this->json_document_.AddMember(rapidjson::Value("output_version", allocator), output_version, allocator);
    rapidjson::StringBuffer str_buf;
    
    if (this->minified_)
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
        this->json_document_.Accept(writer);
    }
    else
    {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buf);
        this->json_document_.Accept(writer);
    }
    
    // Write to output stream if provided, otherwise write to console
    if (this->output_stream_ != nullptr)
    {
        *this->output_stream_ << str_buf.GetString();
    }
    else
    {
        this->options_.GetLog()->WriteStdOut(str_buf.GetString());
    }
}
