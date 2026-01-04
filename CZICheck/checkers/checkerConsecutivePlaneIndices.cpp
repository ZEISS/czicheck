// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerConsecutivePlaneIndices.h"
#include <vector>
#include <memory>
#include <algorithm>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckConsecutivePlaneIndices::kDisplayName = "Check that planes have consecutive indices";
/*static*/const char* CCheckConsecutivePlaneIndices::kShortName = "consecutiveplaneindices";


CCheckConsecutivePlaneIndices::CCheckConsecutivePlaneIndices(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{}

void CCheckConsecutivePlaneIndices::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckConsecutivePlaneIndices::kCheckType);

    this->RunCheckDefaultExceptionHandling([this]()
        { 
            this->CheckForConsecutiveIndices(); 
        });

    this->result_gatherer_.FinishCheck(CCheckConsecutivePlaneIndices::kCheckType);
}

void CCheckConsecutivePlaneIndices::CheckForConsecutiveIndices()
{
    const auto statistics = this->reader_->GetStatistics();

    // This is a map with key "dimension" and value "a bitfield", where we will construct
    //  a bitfield with the size as reported by the statistics. Statistics gives the min and 
    //  the max value for the index, and we create a bitfield of size "max-min".
    map<DimensionIndex, vector<bool>> per_dimension_occupancy;

    statistics.dimBounds.EnumValidDimensions(
        [&](DimensionIndex dim, int start, int size)->bool
        {
            // TODO(JBL): we could/should check for pathological cases like "the size is really large, larger than the number of subblocks",
            //             in which case we can immediately conclude that there has to be a gap
            per_dimension_occupancy.emplace(dim, vector<bool>(size));
            return true;
        });

    // now, just run through the list of subblocks, and "tick away" the reported index in the
    //  bitfield corresponding to the dimension
    this->reader_->EnumerateSubBlocks(
        [&](int index, const SubBlockInfo& info)->bool
        {
            info.coordinate.EnumValidDimensions(
                [&](DimensionIndex dim, int value)->bool
                {
                    // note: we have to subtract the "startIndex" (i.e. the reported minimum for the respective dimension)
                    int startIndex;
                    statistics.dimBounds.TryGetInterval(dim, &startIndex, nullptr);

                    // note: I'd think we can rightfully assume that the dimension is found in the map, and that the 
                    //        index (value - startIndex) is within range. More wary fellows might be more defensive and
                    //        prepare themselves for this assumption to not hold...
                    per_dimension_occupancy[dim][value - startIndex] = true;
                    return true;
                });

            return true;
        });

    // ok, now we can simply check the bitfields, if we find a "false" in there, this means, that there is gap, the
    //  indices are not consecutive
    for (const auto& occupancy : per_dimension_occupancy)
    {
        if (find(occupancy.second.cbegin(), occupancy.second.cend(), false) != occupancy.second.cend())
        {
            IResultGatherer::Finding finding(CCheckConsecutivePlaneIndices::kCheckType);
            finding.severity = IResultGatherer::Severity::Warning;
            stringstream ss;
            ss << "The indices for dimension '" << Utils::DimensionToChar(occupancy.first) << "' are not consecutive";
            finding.information = ss.str();

            // TODO(JBL): we could report the indices missing, maybe as "finding.details" - however, I'd think the "details"-field 
            //             is currently not being outputted, probably we'd want a commandline-options letting us choose whether we
            //             want to see "details" or not.

            this->ThrowIfFindingResultIsStop(this->result_gatherer_.ReportFinding(finding));
        }
    }
}
