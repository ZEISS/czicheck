// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checkerTopographyApplianceValidation.h"
#include <codecvt>

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
        this->CheckTopographySectionExisting(czi_metadata);

        this->ExtractMetaDataDimensions(czi_metadata);
    }

    this->result_gatherer_.FinishCheck(CCheckTopgraphyApplianceMetadata::kCheckType);
}

void CCheckTopgraphyApplianceMetadata::CheckTopographySectionExisting(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata)
{
    string appliance_path{ this->kImageAppliancePath };
    auto metadata_node = czi_metadata->GetChildNodeReadonly(this->kImageAppliancePath);

    if (!metadata_node)
    {
        // There is no Appliances section and we do not need to report that
        return;
    }

    appliance_path
        .append("/Appliance[Id=")
        .append(this->kTopographyItemId)
        .append("]");

    metadata_node = czi_metadata->GetChildNodeReadonly(appliance_path.c_str());

    if (!metadata_node)
    {
        CResultGatherer::Finding finding(CCheckTopgraphyApplianceMetadata::kCheckType);
        finding.severity = CResultGatherer::Severity::Info;
        finding.information = "The ImageDocument does not contain a Topography section in the metadata.";
        this->result_gatherer_.ReportFinding(finding);

        return;
    }
}

void CCheckTopgraphyApplianceMetadata::ExtractMetaDataDimensions(const std::shared_ptr<libCZI::ICziMetadata>& czi_metadata)
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
        return;
    }

    vector<vector<pair<wstring, wstring>>> heightmaps;
    vector<vector<pair<wstring, wstring>>> textures;

    // we need a "named" lambda here to call it recursively
    std::function<bool(std::shared_ptr < libCZI::IXmlNodeRead>)> enumChildrenLabmda =
        [this, &heightmaps, &textures, &enumChildrenLabmda](std::shared_ptr<libCZI::IXmlNodeRead> xmlnode) -> bool
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
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

            auto node_name = utf8_conv.to_bytes(xmlnode->Name());

            if (node_name == "Texture")
            {
                xmlnode->EnumAttributes(textureLambda);

                if (!current_texture.empty())
                {
                    textures.push_back(current_texture);
                }
            }

            if (node_name == "HeightMap")
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
        char dim{ (char)element.first.back() };
        
        configurations.insert({ dim, DimensionView() });
        int value{ -1 };
        try {
            value = stoi(element.second);
        }
        catch (invalid_argument)
        {
            // this will ensure an "invalid" dimension later
            value = -1;
        }
        
        DimensionView* config = &configurations.at(dim);
        if (config->DimensionIndex == DimensionIndex::invalid)
        {
            configurations.at(dim).DimensionIndex = Utils::CharToDimension(dim);
        }

        // -1 means not set
        if (element.first.find(kStart) != string::npos && config->Start == -1)
        {
            config->Start = value;
        }

        if (element.first.find(kSize) != string::npos && config->Size == -1)
        {
            config->Size = value;
        }

        // '0' means not set
        if (config->DimensionName == '0')
        {
            config->DimensionName = dim;
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
