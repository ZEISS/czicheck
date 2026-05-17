// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <libCZI.h>
#include "checkerbase.h"

/// This checker checks whether all subblocks on pyramid layer 0 have consecutive m-indices.
class CCheckMIndicesAreConsecutive : public IChecker, CCheckerBase
{
public:
    static const CZIChecks kCheckType = CZIChecks::MIndicesAreConsecutive;
    static const char* kDisplayName;
    static const char* kShortName;

    CCheckMIndicesAreConsecutive(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGathererReport& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;

private:
    struct SubBlockIndexAndMIndex
    {
        int subblock_index;
        int m_index;
    };

    /// Collects all pyramid-layer-0 subblocks whose coordinate matches the specified plane.
    /// A subblock is considered to be on pyramid layer 0 when its logical rectangle dimensions
    /// equal its physical size.
    /// If any matching subblock has an invalid M-index the enumeration is stopped immediately
    /// and std::nullopt is returned; in that case the caller should not attempt to check
    /// M-index consecutivity for this plane.
    ///
    /// \param  plane_coordinate    The plane coordinate to match against. Only dimensions that
    ///                             are valid in this coordinate are compared; additional dimensions
    ///                             present in a subblock's coordinate are ignored.
    ///
    /// \returns    A vector of SubBlockIndexAndMIndex entries for all matching subblocks, or
    ///             std::nullopt if one or more matching subblocks have an invalid M-index.
    std::optional<std::vector<SubBlockIndexAndMIndex>> GetSubBlocksForPlane(const libCZI::CDimCoordinate& plane_coordinate);
    void CheckMIndicesAreConsecutiveForPlane(const libCZI::CDimCoordinate& plane_coordinate);
};
