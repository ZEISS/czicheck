// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "resultgatherer.h"
#include "checkerfactory.h"
#include "checks.h"
#include "cmdlineoptions.h"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include <algorithm>
#include <ostream>
#include <sstream>
#include <utility>

using namespace std;

CResultGatherer::CResultGatherer(const CCmdLineOptions& options)
    : options_(options)
{
    this->json_document_.SetArray();
    this->test_results_ = rapidjson::Value(rapidjson::kArrayType);
}

void CResultGatherer::StartCheckJson(CZIChecks check)
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
}

void CResultGatherer::StartCheck(CZIChecks check)
{
    if (this->options_.GetEncodingType() == CCmdLineOptions::EncodingType::JSON)
    {
        this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
        this->StartCheckJson(check);
        return;
    }

    const auto checker_display_name = CCheckerFactory::GetCheckerDisplayName(check);
    ostringstream ss;
    ss << "Test \"" << checker_display_name << "\" :";
    this->options_.GetLog()->WriteStdOut(ss.str());

    this->results_.insert(pair<CZIChecks, CheckResult>(check, CheckResult()));
}

void CResultGatherer::FinishCheck(CZIChecks check)
{
    const auto& it = this->results_.find(check);

    const auto& result = it->second;

    if (this->options_.GetEncodingType() == CCmdLineOptions::EncodingType::JSON)
    {
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

        return;
    }

    if (this->options_.GetMaxNumberOfMessagesToPrint() > 0)
    {
        const auto no_of_total_findings = result.GetTotalMessagesCount();
        if (no_of_total_findings > this->options_.GetMaxNumberOfMessagesToPrint())
        {
            const auto findings_omitted = no_of_total_findings - max(this->options_.GetMaxNumberOfMessagesToPrint(), 0);
            ostringstream ss;
            ss << "  <" << findings_omitted << " more finding" << (findings_omitted > 1 ? "s" : "") << " omitted> \n";
            this->options_.GetLog()->WriteStdOut(ss.str());
        }
    }

    if (result.fatalMessagesCount == 0 && result.warningMessagesCount == 0)
    { 
        this->options_.GetLog()->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
        this->options_.GetLog()->WriteStdOut(" OK\n");
        this->options_.GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
    else if (result.fatalMessagesCount == 0)
    {
        this->options_.GetLog()->SetColor(ConsoleColor::LIGHT_RED, ConsoleColor::DEFAULT);
        this->options_.GetLog()->WriteStdOut(" WARN\n");
        this->options_.GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
    else
    {
        this->options_.GetLog()->SetColor(ConsoleColor::DARK_RED, ConsoleColor::DEFAULT);
        this->options_.GetLog()->WriteStdOut(" FAIL\n");
        this->options_.GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
}

void CResultGatherer::ReportFinding(const Finding& finding)
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

    if (this->options_.GetEncodingType() == CCmdLineOptions::EncodingType::JSON)
    {
        writeFinding(finding);
        return;
    }

    if (this->options_.GetMaxNumberOfMessagesToPrint() < 0 ||
        no_of_findings_so_far < this->options_.GetMaxNumberOfMessagesToPrint())
    {
        if (no_of_findings_so_far == 0)
        {
            this->options_.GetLog()->WriteStdOut("\n");
        }

        this->options_.GetLog()->WriteStdOut("  ");
        this->options_.GetLog()->WriteStdOut(finding.information);
        this->options_.GetLog()->WriteStdOut("\n");
        if (this->options_.GetPrintDetailsOfMessages() && !finding.details.empty())
        {
            this->options_.GetLog()->WriteStdOut("  details: ");
            this->options_.GetLog()->SetColor(ConsoleColor::LIGHT_YELLOW, ConsoleColor::DEFAULT);
            this->options_.GetLog()->WriteStdOut(finding.details);
            this->options_.GetLog()->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
            this->options_.GetLog()->WriteStdOut("\n");
        }
    }
}

CResultGatherer::AggregatedResult CResultGatherer::GetAggregatedResult()
{
    std::uint32_t total_fatal_messages_count = 0;
    std::uint32_t total_warning_messages_count = 0;
    std::uint32_t total_info_messages_count = 0;
    for (auto const& i : this->results_)
    {
        total_fatal_messages_count += i.second.fatalMessagesCount;
        total_warning_messages_count += i.second.warningMessagesCount;
        total_info_messages_count += i.second.warningMessagesCount;
    }

    if (total_fatal_messages_count > 0)
    {
        return AggregatedResult::ErrorsDetected;
    }

    if (total_warning_messages_count > 0)
    {
        return AggregatedResult::WithWarnings;
    }

    return AggregatedResult::OK;
}

/*static*/void CResultGatherer::IncrementCounter(Severity severity, CheckResult& result)
{
    switch (severity)
    {
    case Severity::Fatal:
        ++result.fatalMessagesCount;
        break;
    case Severity::Warning:
        ++result.warningMessagesCount;
        break;
    case Severity::Info:
        ++result.infoMessagesCount;
        break;
    }
}

void CResultGatherer::UglyReportFinalHack()
{
    this->json_document_.SetObject();
    auto allocator = this->json_document_.GetAllocator();

    ostringstream ss;
    switch (this->GetAggregatedResult()) {
        case CResultGatherer::AggregatedResult::WithWarnings:
            ss << "WARN";
            break;
        case CResultGatherer::AggregatedResult::ErrorsDetected:
            ss << "FAIL";
            break;
        default:
            ss << "OK";
    }

    this->json_document_.AddMember("aggregatedresult", rapidjson::Value().SetString(ss.str().c_str(), allocator), allocator);
    this->json_document_.AddMember("tests", this->test_results_, allocator);

    rapidjson::StringBuffer str_buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
    this->json_document_.Accept(writer);
    // rapidjson::StringBuffer str_buf;
    // rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
    // this->test_results_.Accept(writer);

    this->options_.GetLog()->WriteStdOut(str_buf.GetString());
}