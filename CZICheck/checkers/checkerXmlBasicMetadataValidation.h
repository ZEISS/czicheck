// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>

/// This checker performs some basic checks on the XML-metadata.
class CCheckBasicMetadataValidation : public IChecker, CCheckerBase
{
public:
	static const CZIChecks kCheckType = CZIChecks::BasicMetadataValidation;
	static const char* kDisplayName;
	static const char* kShortName;

	CCheckBasicMetadataValidation(
		const std::shared_ptr<libCZI::ICZIReader>& reader,
		CResultGatherer& result_gatherer,
		const CheckerCreateInfo& additional_info);
	void RunCheck() override;
private:
    /// Check whether the information at "Metadata/Information/Image/SizeX or StartX" matches the
    /// subblock-statistics.
    ///
    /// \param  doc_info The libCZI-document-info object.
	void CheckSizeInformation(const std::shared_ptr<libCZI::ICziMultiDimensionDocumentInfo>& doc_info) const;

    /// Check the channel information (Metadata/Information/Dimensions/Channels).
    ///
    /// \param  doc_info The libCZI-document-info object.
	void CheckChannelInformation(const std::shared_ptr<libCZI::ICziMultiDimensionDocumentInfo>& doc_info) const;

    /// Check that the pixel type information in "Metadata/Information/Dimensions/Channels" agrees to
    /// the pixeltype found in the actual subblock. Currently, the pixeltype of an arbitrary subblock
    /// (of a given channel) is checked (but there is another checker which verifies that all subblocks
    /// of a given channel have the same pixeltype).
    ///
    /// \param  metadata    The libCZI-document-info object.
	void CheckPixelTypeInformation(const std::shared_ptr<libCZI::ICziMetadata>& metadata) const;
};
