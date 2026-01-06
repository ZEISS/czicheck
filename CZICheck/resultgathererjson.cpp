// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgathererjson.h"
#include "checkerfactory.h"
#include "utils.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include <ostream>
#include <string>
#include <utility>

using namespace std;

const char* CResultGathererJson::kTestNameId = "name";
const char* CResultGathererJson::kTestContainerId = "tests";
const char* CResultGathererJson::kTestDescriptionId = "description";
const char* CResultGathererJson::kTestResultId = "result";
const char* CResultGathererJson::kTestFindingsId = "findings";
const char* CResultGathererJson::kTestSeverityId = "severity";
const char* CResultGathererJson::kTestDetailsId = "details";
const char* CResultGathererJson::kTestAggregationId = "aggregatedresult";
const char* CResultGathererJson::kTestFailFastId = "fail_fast_stopped";

CResultGathererJson::CResultGathererJson(const CCmdLineOptions& options)
    : ResultGathererBase(options)
{
    this->json_document_.SetArray();
    this->test_results_ = rapidjson::Value(rapidjson::kArrayType);
}

void CResultGathererJson::StartCheck(CZIChecks check)
{
    this->CoreStartCheck(check);

    const auto checker_display_name = CCheckerFactory::GetCheckerDisplayName(check).c_str();

    auto allocator = this->json_document_.GetAllocator();

    rapidjson::Value test_run(rapidjson::kObjectType);
    test_run.AddMember(rapidjson::Value(kTestNameId, allocator), rapidjson::Value().SetString(CZIChecksToString(check), allocator), allocator);
    test_run.AddMember(rapidjson::Value(kTestDescriptionId, allocator), rapidjson::Value().SetString(checker_display_name, allocator), allocator);
    test_run.AddMember(rapidjson::Value(kTestResultId, allocator), rapidjson::Value(rapidjson::kStringType), allocator);
    test_run.AddMember(rapidjson::Value(kTestFindingsId, allocator), rapidjson::Value(rapidjson::kArrayType), allocator);

    this->test_results_.PushBack(test_run, allocator);
    this->current_checker_id = std::string(CZIChecksToString(check));
}

void CResultGathererJson::FinishCheck(CZIChecks check)
{
    const IResultGatherer::CheckResult current_checker_result = this->GetCheckResultForCurrentlyActiveChecker();
    this->CoreFinishCheck(check);

    auto allocator = this->json_document_.GetAllocator();
    for (int res { 0 }; res < this->test_results_.Size(); ++res)
    {
        if (this->test_results_[res][kTestNameId].GetString() == this->current_checker_id)
        {
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

            this->test_results_[res][kTestResultId].SetString(ss.str().c_str(), allocator);
        }
    }
}

IResultGatherer::ReportFindingResult CResultGathererJson::ReportFinding(const Finding& finding)
{
    this->CoreReportFinding(finding);

    auto allocator = this->json_document_.GetAllocator();
    for (int res { 0 }; res < this->test_results_.Size(); ++res)
    {
        if (this->test_results_[res][kTestNameId].GetString() == this->current_checker_id)
        {
            rapidjson::Value current_finding(rapidjson::kObjectType);
            current_finding.SetObject()
                .AddMember(rapidjson::Value(kTestSeverityId, allocator), rapidjson::Value().SetString(ResultGathererBase::FindingSeverityToString(finding), allocator), allocator)
                .AddMember(rapidjson::Value(kTestDescriptionId, allocator), rapidjson::Value().SetString(finding.information.c_str(), allocator), allocator)
                .AddMember(rapidjson::Value(kTestDetailsId, allocator), rapidjson::Value().SetString(finding.details.c_str(), allocator), allocator);
            this->test_results_[res][kTestFindingsId].PushBack(current_finding, allocator);
        }
    }

    return this->DetermineReportFindingResult(finding);
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
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buf);

    this->json_document_.Accept(writer);
    this->GetLog()->WriteStdOut(str_buf.GetString());
}

IResultGatherer::CheckResult CResultGathererJson::GetAggregatedCounts() const
{
    return this->CoreGetAggregatedCounts();
}
