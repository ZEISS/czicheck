// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerTopographyApplianceValidation.h"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>
#include <functional>
#include <string>
#include <vector>

using namespace std;
using namespace libCZI;

/*static*/const char* CCheckTopographyApplianceMetadata::kDisplayName = "Basic semantic checks for TopographyDataItems";
/*static*/const char* CCheckTopographyApplianceMetadata::kShortName = "topographymetadata";

CCheckTopographyApplianceMetadata::CCheckTopographyApplianceMetadata(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    IResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckTopographyApplianceMetadata::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckTopographyApplianceMetadata::kCheckType);

    const auto czi_metadata = this->GetCziMetadataAndReportErrors(CCheckTopographyApplianceMetadata::kCheckType);
    if (czi_metadata)
    {
        this->CheckValidDimensionInTopographyDataItems(czi_metadata);
    }

    this->result_gatherer_.FinishCheck(CCheckTopographyApplianceMetadata::kCheckType);
}

void CCheckTopographyApplianceMetadata::CheckValidDimensionInTopographyDataItems(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata)
{
    if (!this->ExtractMetaDataDimensions(czi_metadata))
    {
        // we don't have any topography items and stop here
        return;
    }

    if (this->texture_views_.empty() || this->heightmap_views_.empty())
    {
        IResultGatherer::Finding finding(CCheckTopographyApplianceMetadata::kCheckType);
        finding.severity = IResultGatherer::Severity::Warning;
        finding.information = "The image contains incomplete TopographyDataItems.";
        this->result_gatherer_.ReportFinding(finding);

        return;
    }

    // as soon as we have more than StartC specified for a Texutre or a Heightmap node
    // the node contains superfluous data
    auto superfluous_elements_check = [](const unordered_map<char, DimensionView>& dim_map) -> bool
        {
            if (dim_map.size() != 1)
            {
                return false;
            }

            return true;
        };

    auto start_c_defined_check = [](const unordered_map<char, DimensionView>& dim_map) -> bool
        {
            for (const auto& dim : dim_map)
            {
                if (dim.second.DimensionIndex == DimensionIndex::C
                    && dim.second.IsValid())
                    return true;
            }

            return false;
        };

    unordered_map<int, bool> c_indices_set;
    bool superfluous_free{ true };
    bool start_c_defined{ true };
    for (const auto& txt : this->texture_views_)
    {
        superfluous_free &= superfluous_elements_check(txt);
        start_c_defined &= start_c_defined_check(txt);
        if (!start_c_defined)
        {
            continue;
        }

        // arriving here, the channel indices left are valid and can be added to a set of channel indices
        for (const auto& el : txt)
        {
            if (el.second.DimensionIndex == libCZI::DimensionIndex::C)
            {
                c_indices_set.insert({el.second.Start, false});
            }
        }
    }

    for (const auto& hmp : this->heightmap_views_)
    {
        superfluous_free &= superfluous_elements_check(hmp);
        start_c_defined &= start_c_defined_check(hmp);
        if (!start_c_defined)
        {
            continue;
        }

        // arriving here, the channel indices left are valid and can be added to a set of channel indices
        for (const auto& el : hmp)
        {
            if (el.second.DimensionIndex == libCZI::DimensionIndex::C)
            {
                c_indices_set.insert({el.second.Start, false});
            }
        }
    }

    if (!superfluous_free)
    {
        IResultGatherer::Finding finding(CCheckTopographyApplianceMetadata::kCheckType);
        finding.severity = IResultGatherer::Severity::Warning;
        finding.information = "There are superfluous dimensions specified in the TopographyDataItems. This might yield errors.";
        this->result_gatherer_.ReportFinding(finding);
    }

    if (!start_c_defined)
    {
        IResultGatherer::Finding finding(CCheckTopographyApplianceMetadata::kCheckType);
        finding.severity = IResultGatherer::Severity::Fatal;
        finding.information = "The image contains TopographyDataItems that do not define a channel.";
        this->result_gatherer_.ReportFinding(finding);
    }

    if (!CheckExistenceOfSpecifiedChannels(c_indices_set))
    {
        IResultGatherer::Finding finding(CCheckTopographyApplianceMetadata::kCheckType);
        finding.severity = IResultGatherer::Severity::Fatal;
        finding.information = "The Topography metadata specifies channels for the texture or heightmap subblocks, that are not present in the Subblock Collection of the image.";
        this->result_gatherer_.ReportFinding(finding);
    }
}

bool CCheckTopographyApplianceMetadata::ExtractMetaDataDimensions(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata)
{
    // within the TopographyData we allow
    // any number of TopographyDataItem which itself can contain a set of Textures and a set of Heightmaps
    // within the heightmaps AND Textures, each item reside in its own channel.
    string topography_path{ CCheckTopographyApplianceMetadata::kImageAppliancePath };
    topography_path
        .append("/Appliance[Id=")
        .append(CCheckTopographyApplianceMetadata::kTopographyItemId)
        .append("]");

    const auto topo_metadata{ czi_metadata->GetChildNodeReadonly(topography_path.c_str()) };

    if (!topo_metadata)
    {
        // there is no topo metadata section, we end here
        return false;
    }

    vector<vector<pair<wstring, wstring>>> heightmaps;
    vector<vector<pair<wstring, wstring>>> textures;

    // we need a "named" lambda here to call it recursively
    std::function<bool(std::shared_ptr < libCZI::IXmlNodeRead>)> enumChildrenLambda =
        [this, &heightmaps, &textures, &enumChildrenLambda](const std::shared_ptr<libCZI::IXmlNodeRead>& xmlnode) -> bool
        {
            std::vector<std::pair<std::wstring, std::wstring>> current_texture;
            std::vector<std::pair<std::wstring, std::wstring>> current_heightmap;

            auto textureLambda = [&current_texture](const std::wstring& attr, const std::wstring& val) -> bool
                {
                    current_texture.push_back({ attr, val });
                    return true;
                };

            auto heighmapLambda = [&current_heightmap](const std::wstring& attr, const std::wstring& val) -> bool
                {
                    current_heightmap.push_back({ attr, val });
                    return true;
                };

            auto node_name = xmlnode->Name();
            if (node_name == CCheckTopographyApplianceMetadata::kTextureItemKey)
            {
                xmlnode->EnumAttributes(textureLambda);

                if (!current_texture.empty())
                {
                    textures.push_back(current_texture);
                }
            }

            if (node_name == CCheckTopographyApplianceMetadata::kHeightMapItemKey)
            {
                xmlnode->EnumAttributes(heighmapLambda);

                if (!current_heightmap.empty())
                {
                    heightmaps.push_back(current_heightmap);
                }
            }

            // recursively go through child items 
            xmlnode->EnumChildren(enumChildrenLambda);

            return true;
        };

    // call the enumeration lambda
    topo_metadata->EnumChildren(enumChildrenLambda);

    // parse the dimension vectors
    for (const auto& hm : heightmaps)
    {
        this->SetBoundsFromVector(hm, this->heightmap_views_);
    }

    for (const auto& tx : textures)
    {
        this->SetBoundsFromVector(tx, this->texture_views_);
    }

    if (!this->heightmap_views_.empty() || !this->texture_views_.empty())
    {
        return true;
    }

    return false;
}

bool CCheckTopographyApplianceMetadata::CheckExistenceOfSpecifiedChannels(std::unordered_map<int, bool>& indices_set)
{
    if (std::all_of(indices_set.cbegin(), indices_set.cend(), [](const auto& el) { return el.second; }))
    {
        return true;
    }

    this->reader_->EnumerateSubBlocks([this, &indices_set](int index, const SubBlockInfo& info) -> bool
    {
        int current_start_c { -1 };
        if (!info.coordinate.TryGetPosition(libCZI::DimensionIndex::C, &current_start_c))
        {
            return true;
        }
        // Here we check if any of the specified (in the topography metadata) channel indices
        // match the StartC index of the current subblock in the subblock collection we are iterating over.
        // When this is the case, we set the corresponding boolean to true, indicating the existence of the
        // channel index it corresponds with (in the dictionary) in the subblock collection.
        for (auto& el : indices_set)
        {
            el.second |= current_start_c == el.first;
        }

        return true;
    });

    return std::all_of(indices_set.cbegin(), indices_set.cend(), [](const auto& el) { return el.second; });
}

bool CCheckTopographyApplianceMetadata::SetBoundsFromVector(const std::vector<std::pair<std::wstring, std::wstring>>& vec, std::vector<std::unordered_map<char, DimensionView>>& view)
{
    // using a set here to ensure exactly one element per dimension
    unordered_map<char, DimensionView> configurations;

    const wstring kStart = L"Start";
    const wstring kSize = L"Size";

    for (const auto& element : vec)
    {
        // get the dimension index
        char dim{ static_cast<char>(element.first.back()) };

        configurations.insert({ dim, DimensionView() });
        int value{ -1 };
        try
        {
            value = stoi(element.second);
        }
        catch (const invalid_argument&)
        {
            // this will ensure an "invalid" dimension later
            value = -1;
        }

        DimensionView& config = configurations.at(dim);
        if (config.DimensionIndex == DimensionIndex::invalid)
        {
            config.DimensionIndex = Utils::CharToDimension(dim);
        }

        // -1 means not set
        if (element.first.find(kStart) != string::npos && config.Start == -1)
        {
            config.Start = value;
        }

        if (element.first.find(kSize) != string::npos && config.Size == -1)
        {
            config.Size = value;
        }

        // '\0' means not set
        if (config.DimensionName == '\0')
        {
            config.DimensionName = dim;
        }
    }

    bool all_good{ true };
    for (const auto& el : configurations)
    {
        all_good &= el.second.IsValid();
    }

    view.push_back(configurations);

    return all_good;
};
