// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerSubBlkBitmapValid.h"
#include <exception>
#include <sstream>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckSubBlkBitmapValid::kDisplayName = "SubBlock-Segments in SubBlockDirectory are valid and valid content";
/*static*/const char* CCheckSubBlkBitmapValid::kShortName = "subblkbitmapvalid";

CCheckSubBlkBitmapValid::CCheckSubBlkBitmapValid(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckSubBlkBitmapValid::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckSubBlkBitmapValid::kCheckType);
    // First enumerate subblock indices so we can process them in parallel and cancel early when desired
    std::vector<int> indices;
    this->reader_->EnumerateSubBlocks([&indices](int index, const SubBlockInfo&)->bool { indices.push_back(index); return true; });

    if (indices.empty())
    {
        this->result_gatherer_.FinishCheck(CCheckSubBlkBitmapValid::kCheckType);
        return;
    }

    unsigned int num_threads = static_cast<unsigned int>(this->additional_info_.subblockThreads);
    if (num_threads == 0) num_threads = 1;
    std::atomic<size_t> next_index(0);
    std::atomic<bool> cancel{ false };
    std::mutex gatherer_mutex;

    auto worker = [&]() {
        while (!cancel.load())
        {
            const size_t i = next_index.fetch_add(1);
            if (i >= indices.size()) break;
            const int index = indices[i];

            try
            {
                auto sub_block = this->reader_->ReadSubBlock(index);
                const auto compression_mode = sub_block->GetSubBlockInfo().GetCompressionMode();
                if (compression_mode != CompressionMode::Invalid)
                {
                    try
                    {
                        auto bitmap = sub_block->CreateBitmap();
                    }
                    catch (exception& exception)
                    {
                        IResultGatherer::Finding finding(CCheckSubBlkBitmapValid::kCheckType);
                        finding.severity = IResultGatherer::Severity::Fatal;
                        stringstream ss;
                        ss << "Error decoding subblock #" << index << " with compression \"" << Utils::CompressionModeToInformalString(compression_mode) << "\"";
                        finding.information = ss.str();
                        finding.details = exception.what();
                        {
                            std::lock_guard<std::mutex> lg(gatherer_mutex);
                            this->result_gatherer_.ReportFinding(finding);
                            if (this->result_gatherer_.IsFailFastEnabled() && this->result_gatherer_.HasFatal(CCheckSubBlkBitmapValid::kCheckType))
                            {
                                this->result_gatherer_.NotifyFailFastStop(CCheckSubBlkBitmapValid::kCheckType);
                                cancel.store(true);
                            }
                        }
                    }
                }
                else
                {
                    IResultGatherer::Finding finding(CCheckSubBlkBitmapValid::kCheckType);
                    finding.severity = IResultGatherer::Severity::Info;
                    stringstream ss;
                    ss << "Subblock #" << index << " has a non-standard compression mode (" << sub_block->GetSubBlockInfo().compressionModeRaw << ")";
                    finding.information = ss.str();
                    std::lock_guard<std::mutex> lg(gatherer_mutex);
                    this->result_gatherer_.ReportFinding(finding);
                }
            }
            catch (exception& exception)
            {
                IResultGatherer::Finding finding(CCheckSubBlkBitmapValid::kCheckType);
                finding.severity = IResultGatherer::Severity::Fatal;
                stringstream ss;
                ss << "Error reading subblock #" << index;
                finding.information = ss.str();
                finding.details = exception.what();
                {
                    std::lock_guard<std::mutex> lg(gatherer_mutex);
                    this->result_gatherer_.ReportFinding(finding);
                    if (this->result_gatherer_.IsFailFastEnabled() && this->result_gatherer_.HasFatal(CCheckSubBlkBitmapValid::kCheckType))
                    {
                        this->result_gatherer_.NotifyFailFastStop(CCheckSubBlkBitmapValid::kCheckType);
                        cancel.store(true);
                    }
                }
            }
        }
    };

    std::vector<std::thread> workers;
    workers.reserve(num_threads);
    for (unsigned int t = 0; t < num_threads; ++t)
    {
        workers.emplace_back(worker);
    }

    for (auto &w : workers) w.join();

    this->result_gatherer_.FinishCheck(CCheckSubBlkBitmapValid::kCheckType);
}

