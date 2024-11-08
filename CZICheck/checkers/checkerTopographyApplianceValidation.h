// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "checkerbase.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <utility>

/// This checker validates the topography-XML-metadata.
class CCheckTopographyApplianceMetadata : public IChecker, CCheckerBase
{
private:
    struct DimensionView
    {
        libCZI::DimensionIndex DimensionIndex{ libCZI::DimensionIndex::invalid };
        char DimensionName{ '\0' };
        int Start{ -1 };
        int Size{ -1 };
        bool IsValid() const
        {
            // this is an object used exclusively for this checker
            //  a Size (SizeC, SizeX, etc.) is not needed to yield a "valid" dimension for this
            if (this->Start >= 0 && this->DimensionIndex != libCZI::DimensionIndex::invalid)
            {
                return true;
            }

            return false;
        }
    };

    static constexpr const char* kTopographyItemId = "Topography:1";
    static constexpr const char* kImageAppliancePath = "ImageDocument/Metadata/Appliances";
    static constexpr const wchar_t* kTextureItemKey = L"Texture";
    static constexpr const wchar_t* kHeightMapItemKey = L"HeightMap";

    std::vector<std::unordered_map<char, DimensionView>> texture_views_;
    std::vector<std::unordered_map<char, DimensionView>> heightmap_views_;

public:
    static const CZIChecks kCheckType = CZIChecks::ApplianceMetadataTopographyItemValid;
    static const char* kDisplayName;
    static const char* kShortName;
    
    CCheckTopographyApplianceMetadata(
        const std::shared_ptr<libCZI::ICZIReader>& reader,
        IResultGatherer& result_gatherer,
        const CheckerCreateInfo& additional_info);
    void RunCheck() override;

private:
    void CheckValidDimensionInTopographyDataItems(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata);

    bool SetBoundsFromVector(const std::vector<std::pair<std::wstring, std::wstring>>&, std::vector<std::unordered_map<char, DimensionView>>&);
    bool CheckExistenceOfSpecifiedChannels(std::unordered_map<int, bool>& indices_set);
    bool ExtractMetaDataDimensions(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata);
};
