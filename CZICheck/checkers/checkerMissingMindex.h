// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <memory>
#include "checkerbase.h"

/// This checker checks whether all subblocks on pyramid layer 0 have an m-index.
class CCheckMissingMindex : public IChecker, CCheckerBase
{
public:
	static const CZIChecks kCheckType = CZIChecks::SubblocksHaveMindex;
	static const char* kDisplayName;
	static const char* kShortName;

	CCheckMissingMindex(
		const std::shared_ptr<libCZI::ICZIReader>& reader,
		CResultGatherer& result_gatherer,
		const CheckerCreateInfo& additional_info);
	void RunCheck() override;
};
