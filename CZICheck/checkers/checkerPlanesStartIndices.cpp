// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerPlanesStartIndices.h"
#include <sstream>
#include <memory>

using namespace libCZI;
using namespace std;

/*static*/const char* CCheckPlanesStartIndices::kDisplayName = "Check that planes indices start at 0";
/*static*/const char* CCheckPlanesStartIndices::kShortName = "planesstartindex";

CCheckPlanesStartIndices::CCheckPlanesStartIndices(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckPlanesStartIndices::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckPlanesStartIndices::kCheckType);

    const auto statistics = this->reader_->GetStatistics();

    /// Enumerates all valid dimensions and checks if they start in 0.
    statistics.dimBounds.EnumValidDimensions(
        [this](DimensionIndex dimIndex, int start, int size)->bool
        {
            if (start != 0)
            {
                CResultGatherer::Finding finding(CCheckPlanesStartIndices::kCheckType);
                finding.severity = CResultGatherer::Severity::Warning;
                stringstream ss;
                ss << "plane indices for '" << Utils::DimensionToChar(dimIndex) <<
                    "' do not start at 0, but at " << start << " instead.";
                finding.information = ss.str();
                this->result_gatherer_.ReportFinding(finding);
            }

            return true;
        });

    this->result_gatherer_.FinishCheck(CCheckPlanesStartIndices::kCheckType);
}
