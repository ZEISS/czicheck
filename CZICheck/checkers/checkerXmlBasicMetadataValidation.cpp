// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerXmlBasicMetadataValidation.h"
#include <algorithm>
#include <memory>
#include <vector>

using namespace std;
using namespace libCZI;

/*static*/const char* CCheckBasicMetadataValidation::kDisplayName = "Basic semantic checks of the XML-metadata";
/*static*/const char* CCheckBasicMetadataValidation::kShortName = "basicxmlmetadata";

CCheckBasicMetadataValidation::CCheckBasicMetadataValidation(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckBasicMetadataValidation::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckBasicMetadataValidation::kCheckType);

    // note: "GetCziMetadataAndReportErrors" will report a warning in case the ICziMetadata-object cannot be constructed
    const auto czi_metadata = this->GetCziMetadataAndReportErrors(CCheckBasicMetadataValidation::kCheckType);
    if (czi_metadata)
    {
        const auto doc_info = czi_metadata->GetDocumentInfo();

        this->CheckSizeInformation(doc_info);
        this->CheckChannelInformation(doc_info);
        this->CheckPixelTypeInformation(czi_metadata);
    }

    this->result_gatherer_.FinishCheck(CCheckBasicMetadataValidation::kCheckType);
}

void CCheckBasicMetadataValidation::CheckSizeInformation(const std::shared_ptr<libCZI::ICziMultiDimensionDocumentInfo>& doc_info) const
{
    vector<DimensionIndex> valid_dimension_in_metadata = doc_info->GetDimensions();

    // first, check whether all dimension which are present in the "statistics" are also present in the XML-metadata
    auto statistics = this->reader_->GetStatistics();
    vector<DimensionIndex> dimensions_from_statistics_not_present_in_metadata;
    statistics.dimBounds.EnumValidDimensions(
        [&](libCZI::DimensionIndex dim, int start, int size)->bool
        {
            if (find(valid_dimension_in_metadata.cbegin(), valid_dimension_in_metadata.cend(), dim) == valid_dimension_in_metadata.cend())
            {
                dimensions_from_statistics_not_present_in_metadata.emplace_back(dim);
            }

            return true;
        });

    if (!dimensions_from_statistics_not_present_in_metadata.empty())
    {
        CResultGatherer::Finding finding(CCheckBasicMetadataValidation::kCheckType);
        finding.severity = CResultGatherer::Severity::Warning;
        stringstream ss;
        ss << "The sizes of the following dimensions (from 'document statistics') are not given in the document's metadata: ";
        bool isFirst = true;
        for (auto d : dimensions_from_statistics_not_present_in_metadata)
        {
            if (!isFirst)
            {
                ss << ',';
            }

            ss << Utils::DimensionToChar(d);
            isFirst = false;
        }

        finding.information = ss.str();
        this->result_gatherer_.ReportFinding(finding);
    }

    // now, check whether the start/size are the same
    vector<DimensionIndex> dimensions_where_start_or_size_differ;
    for (auto dim : valid_dimension_in_metadata)
    {
        int start_index_statistics, size_statistics;
        if (statistics.dimBounds.TryGetInterval(dim, &start_index_statistics, &size_statistics))
        {
            auto startEndFromMetadata = doc_info->GetDimensionInfo(dim);

            int startIndexMetadata, endIndexMetadata;
            startEndFromMetadata->GetInterval(&startIndexMetadata, &endIndexMetadata);

            if (start_index_statistics != startIndexMetadata || endIndexMetadata != start_index_statistics + size_statistics)
            {
                dimensions_where_start_or_size_differ.emplace_back(dim);
            }
        }
    }

    if (!dimensions_where_start_or_size_differ.empty())
    {
        CResultGatherer::Finding finding(CCheckBasicMetadataValidation::kCheckType);
        finding.severity = CResultGatherer::Severity::Warning;
        stringstream ss;
        ss << "For the following dimensions the start/size given in metadata differs from document statistics: ";
        bool isFirst = true;
        for (auto d : dimensions_where_start_or_size_differ)
        {
            if (!isFirst)
            {
                ss << ',';
            }

            ss << Utils::DimensionToChar(d);
            isFirst = false;
        }

        finding.information = ss.str();
        this->result_gatherer_.ReportFinding(finding);
    }
}

void CCheckBasicMetadataValidation::CheckChannelInformation(const std::shared_ptr<libCZI::ICziMultiDimensionDocumentInfo>& doc_info) const
{
    // compare the number of nodes in dimensions/channel to the number of channels in statistics
    int channel_count_from_statistics;
    if (!this->reader_->GetStatistics().dimBounds.TryGetInterval(DimensionIndex::C, nullptr, &channel_count_from_statistics))
    {
        // well... if there is no "C-dimension" in the statistics, then we just skip this test. The absence of C-dimension
        //  is reported with some other checker, so maybe it is a good idea to skip reporting a warning here
        return;
    }

    const auto channel_info = doc_info->GetDimensionChannelsInfo();
    if (!channel_info)
    {
        CResultGatherer::Finding finding(CCheckBasicMetadataValidation::kCheckType);
        finding.severity = CResultGatherer::Severity::Warning;
        finding.information = "No valid channel-information found in metadata";
        this->result_gatherer_.ReportFinding(finding);
        return;
    }

    if (channel_info->GetChannelCount() != channel_count_from_statistics)
    {
        CResultGatherer::Finding finding(CCheckBasicMetadataValidation::kCheckType);
        finding.severity = CResultGatherer::Severity::Warning;
        stringstream ss;
        ss << "document statistics gives " << channel_count_from_statistics << " channels, whereas in XML-metadata " << channel_info->GetChannelCount() << " channels are found.";
        finding.information = ss.str();
        this->result_gatherer_.ReportFinding(finding);
    }
}

void CCheckBasicMetadataValidation::CheckPixelTypeInformation(const std::shared_ptr<libCZI::ICziMetadata>& metadata) const
{
    const auto doc_info = metadata->GetDocumentInfo();

    const auto dimensions_channels_info = doc_info->GetDimensionChannelsInfo();
    if (!dimensions_channels_info)
    {
        CResultGatherer::Finding finding(CCheckBasicMetadataValidation::kCheckType);
        finding.severity = CResultGatherer::Severity::Info;
        finding.information = "No valid channel-information found in metadata";
        this->result_gatherer_.ReportFinding(finding);
        return;
    }

    for (int channelIndex = 0; channelIndex < dimensions_channels_info->GetChannelCount(); channelIndex++)
    {
        const auto channel_info = dimensions_channels_info->GetChannel(channelIndex);
        PixelType channel_info_pixel_type;
        if (channel_info->TryGetPixelType(&channel_info_pixel_type))
        {
            SubBlockInfo subBlockInfo;
            // there's another checker for pixelType consistency for all subBlocks in a channel, but here we validate the metadata information
            if (reader_->TryGetSubBlockInfoOfArbitrarySubBlockInChannel(channelIndex, subBlockInfo))
            {
                if (subBlockInfo.pixelType != channel_info_pixel_type)
                {
                    CResultGatherer::Finding finding(CCheckBasicMetadataValidation::kCheckType);
                    finding.severity = CResultGatherer::Severity::Warning;
                    stringstream ss;
                    ss << "PixelType mismatch between metadata and sub block-information. "
                        << "channel index: " << channelIndex
                        << ", metadata: " << Utils::PixelTypeToInformalString(channel_info_pixel_type)
                        << ", subBlock: " << Utils::PixelTypeToInformalString(subBlockInfo.pixelType);
                    finding.information = ss.str();
                    this->result_gatherer_.ReportFinding(finding);
                }
            }
            else
            {
                CResultGatherer::Finding finding(CCheckBasicMetadataValidation::kCheckType);
                finding.severity = CResultGatherer::Severity::Info;
                stringstream ss;
                ss << "No sub block-information found for channel index " << channelIndex << ", metadata pixelType: " << Utils::PixelTypeToInformalString(channel_info_pixel_type);
                finding.information = ss.str();
                this->result_gatherer_.ReportFinding(finding);
            }
        }
        else
        {
            CResultGatherer::Finding finding(CCheckBasicMetadataValidation::kCheckType);
            finding.severity = CResultGatherer::Severity::Info;
            finding.information = "No valid channel-information found in metadata";
            this->result_gatherer_.ReportFinding(finding);
        }
    }
}
