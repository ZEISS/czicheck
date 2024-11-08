// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererjson.h"
#include "checkerfactory.h"

#include <ostream>
#include <string>

using namespace std;

CResultGathererJson::CResultGathererJson(const CCmdLineOptions& options)
    : options_(options)
{
    this->json_document_.SetArray();
    this->test_results_ = rapidjson::Value(rapidjson::kArrayType);
}


void CResultGathererJson::StartCheck(CZIChecks check)
{
    const auto add_test = [&](const char* test_name, const char* display_name) -> void
    {
        auto allocator = this->json_document_.GetAllocator();
        rapidjson::Value test_run(rapidjson::kObjectType);
        test_run.AddMember("test", rapidjson::Value().SetString(test_name, allocator), allocator);
        test_run.AddMember("description", rapidjson::Value().SetString(display_name, allocator), allocator);
        test_run.AddMember("result", rapidjson::Value(rapidjson::kStringType), allocator);
        test_run.AddMember("findings", rapidjson::Value(rapidjson::kArrayType), allocator);
        this->test_results_.PushBack(test_run, allocator);
        this->current_checker_id = std::string(test_name);
    };

    const auto checker_display_name = CCheckerFactory::GetCheckerDisplayName(check).c_str();
    add_test(CZIChecksToString(check), checker_display_name);

    this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
}

void CResultGathererJson::FinishCheck(CZIChecks check)
{
    const auto& it = this->results_.find(check);
    const auto& result = it->second;

    const auto writeFinding = [&]() -> bool
    {
        auto allocator = this->json_document_.GetAllocator();
        bool found_key { false };
        for (int res { 0 }; res < this->test_results_.Size(); ++res)
        {
            if (this->test_results_[res]["test"].GetString() == this->current_checker_id)
            {
                found_key = true;
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

                this->test_results_[res]["result"].SetString(ss.str().c_str(), allocator);
            }
        }

        return found_key;
    };

    writeFinding();
}

void CResultGathererJson::ReportFinding(const Finding& finding)
{
    const auto it = this->results_.find(finding.check);
    const auto no_of_findings_so_far = it->second.GetTotalMessagesCount();
    IncrementCounter(finding.severity, it->second);

    const auto writeFinding = [&](const Finding& finding) -> bool
    {
        auto allocator = this->json_document_.GetAllocator();
        bool found_key { false };
        for (int res { 0 }; res < this->test_results_.Size(); ++res)
        {
            if (this->test_results_[res]["test"].GetString() == this->current_checker_id)
            {
                found_key = true;
                rapidjson::Value current_finding(rapidjson::kObjectType);
                current_finding.SetObject()
                    .AddMember("severity", rapidjson::StringRef(finding.FindingSeverityToString()), allocator)
                    .AddMember("description", rapidjson::Value().SetString(finding.information.c_str(), allocator), allocator)
                    .AddMember("details", rapidjson::Value().SetString(finding.details.c_str(), allocator), allocator);
                this->test_results_[res]["findings"].PushBack(current_finding, allocator);
            }
        }

        return found_key;
    };

    writeFinding(finding);
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

    this->json_document_.AddMember("aggregatedresult", rapidjson::Value().SetString(ss.str().c_str(), allocator), allocator);
    this->json_document_.AddMember("tests", this->test_results_, allocator);

    rapidjson::StringBuffer str_buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buf);
    // rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);

    this->json_document_.Accept(writer);
    this->options_.GetLog()->WriteStdOut(str_buf.GetString());
}