// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerSubBlkSegmentsValid.h"
#include <exception>
#include <sstream>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckSubBlkSegmentsValid::kDisplayName = "SubBlock-Segments in SubBlockDirectory are valid";
/*static*/const char* CCheckSubBlkSegmentsValid::kShortName = "subblksegmentsvalid";

CCheckSubBlkSegmentsValid::CCheckSubBlkSegmentsValid(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckSubBlkSegmentsValid::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckSubBlkSegmentsValid::kCheckType);
    // Gather indices first so we can parallelize reads and cancel early on failure
    std::vector<int> indices;
    this->reader_->EnumerateSubBlocks([&indices](int index, const SubBlockInfo&)->bool { indices.push_back(index); return true; });

    if (indices.empty())
    {
        this->result_gatherer_.FinishCheck(CCheckSubBlkSegmentsValid::kCheckType);
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
                this->reader_->ReadSubBlock(index);
            }
            catch (exception& exception)
            {
                IResultGatherer::Finding finding(CCheckSubBlkSegmentsValid::kCheckType);
                finding.severity = IResultGatherer::Severity::Fatal;
                stringstream ss;
                ss << "Error reading subblock #" << index;
                finding.information = ss.str();
                finding.details = exception.what();
                {
                    std::lock_guard<std::mutex> lg(gatherer_mutex);
                    this->result_gatherer_.ReportFinding(finding);
                    if (this->result_gatherer_.IsFailFastEnabled() && this->result_gatherer_.HasFatal(CCheckSubBlkSegmentsValid::kCheckType))
                    {
                        this->result_gatherer_.NotifyFailFastStop(CCheckSubBlkSegmentsValid::kCheckType);
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

    this->result_gatherer_.FinishCheck(CCheckSubBlkSegmentsValid::kCheckType);
}
