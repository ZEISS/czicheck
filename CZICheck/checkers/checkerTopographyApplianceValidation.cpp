// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerTopographyApplianceValidation.h"
#include <codecvt>
#include <memory>
#include <unordered_map>
#include <utility>

using namespace std;
using namespace libCZI;

/*static*/const char* CCheckTopgraphyApplianceMetadata::kDisplayName = "Basic semantic checks for TopographyDataItems";
/*static*/const char* CCheckTopgraphyApplianceMetadata::kShortName = "topographymetadata";

CCheckTopgraphyApplianceMetadata::CCheckTopgraphyApplianceMetadata(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info) :
    CCheckerBase(reader, result_gatherer, additional_info)
{
}

void CCheckTopgraphyApplianceMetadata::RunCheck()
{
    this->result_gatherer_.StartCheck(CCheckTopgraphyApplianceMetadata::kCheckType);

    const auto czi_metadata = this->GetCziMetadataAndReportErrors(CCheckTopgraphyApplianceMetadata::kCheckType);
    if (czi_metadata)
    {
        this->CheckValidDimensionInTopographyDataItems(czi_metadata);
    }

    this->result_gatherer_.FinishCheck(CCheckTopgraphyApplianceMetadata::kCheckType);
}

void CCheckTopgraphyApplianceMetadata::CheckValidDimensionInTopographyDataItems(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata)
{
    if (!this->ExtractMetaDataDimensions(czi_metadata))
    {
        // we don't have any topography items and stop here
        return;
    }

    if (this->texture_views.empty() || this->heightmap_views.empty())
    {
        CResultGatherer::Finding finding(CCheckTopgraphyApplianceMetadata::kCheckType);
        finding.severity = CResultGatherer::Severity::Warning;
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

    bool superfluous_free{ true };
    bool start_c_defined{ true };
    for (const auto& txt : this->texture_views)
    {
        superfluous_free &= superfluous_elements_check(txt);
        start_c_defined &= start_c_defined_check(txt);
    }

    for (const auto& hmp : this->heightmap_views)
    {
        superfluous_free &= superfluous_elements_check(hmp);
        start_c_defined &= start_c_defined_check(hmp);
    }

    if (!superfluous_free)
    {
        CResultGatherer::Finding finding(CCheckTopgraphyApplianceMetadata::kCheckType);
        finding.severity = CResultGatherer::Severity::Warning;
        finding.information = "There are superfluous dimensions specified in the TopographyDataItems. This might yield errors.";
        this->result_gatherer_.ReportFinding(finding);
    }

    if (!start_c_defined)
    {
        CResultGatherer::Finding finding(CCheckTopgraphyApplianceMetadata::kCheckType);
        finding.severity = CResultGatherer::Severity::Fatal;
        finding.information = "The image contains TopographyDataItems that do not define a channel.";
        this->result_gatherer_.ReportFinding(finding);
    }
}

bool CCheckTopgraphyApplianceMetadata::ExtractMetaDataDimensions(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata)
{
    // within the TopographyData we allow
    // any number of TopographyDataItem which itself can contain a set of Texutures and a set of heightmaps
    // within the heightmaps AND Textures, each item reside in its own channel.
    string topography_path{ this->kImageAppliancePath };
    topography_path
        .append("/Appliance[Id=")
        .append(this->kTopographyItemId)
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
    std::function<bool(std::shared_ptr < libCZI::IXmlNodeRead>)> enumChildrenLabmda =
        [this, &heightmaps, &textures, &enumChildrenLabmda](const std::shared_ptr<libCZI::IXmlNodeRead> xmlnode) -> bool
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
            if (node_name == CCheckTopgraphyApplianceMetadata::kTextureItemKey)
            {
                xmlnode->EnumAttributes(textureLambda);

                if (!current_texture.empty())
                {
                    textures.push_back(current_texture);
                }
            }

            if (node_name == CCheckTopgraphyApplianceMetadata::kHeighMapItemKey)
            {
                xmlnode->EnumAttributes(heighmapLambda);

                if (!current_heightmap.empty())
                {
                    heightmaps.push_back(current_heightmap);
                }
            }

            // recursively go through child items 
            xmlnode->EnumChildren(enumChildrenLabmda);

            return true;
        };

    // call the enumeration lambda
    topo_metadata->EnumChildren(enumChildrenLabmda);

    // parse the dimension vectors
    for (const auto& hm : heightmaps)
    {
        this->SetBoundsFromVector(hm, this->heightmap_views);
    }

    for (const auto& tx : textures)
    {
        this->SetBoundsFromVector(tx, this->texture_views);
    }

    if (!this->heightmap_views.empty() || !this->texture_views.empty())
    {
        return true;
    }

    return false;
}


bool CCheckTopgraphyApplianceMetadata::SetBoundsFromVector(const std::vector<std::pair<std::wstring, std::wstring>>& vec, std::vector<std::unordered_map<char, DimensionView>>& view)
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
            configurations.at(dim).DimensionIndex = Utils::CharToDimension(dim);
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

        // '0' means not set
        if (config.DimensionName == '0')
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
