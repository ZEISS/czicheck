// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "checks.h"

const char* CZIChecksToString(CZIChecks czi_check)
{
    switch (czi_check)
    {
        case CZIChecks::SubBlockDirectoryPositionsWithinRange: return "SubBlockDirectoryPositionsWithinRange";
        case CZIChecks::SubBlockDirectorySegmentValid: return "SubBlockDirectorySegmentValid";
        case CZIChecks::ConsistentSubBlockCoordinates: return "ConsistentSubBlockCoordinates";
        case CZIChecks::DuplicateSubBlockCoordinates: return "DuplicateSubBlockCoordinates";
        case CZIChecks::BenabledDocument: return "BenabledDocument";
        case CZIChecks::SamePixeltypePerChannel: return "SamePixeltypePerChannel";
        case CZIChecks::PlanesIndicesStartAtZero: return "PlanesIndicesStartAtZero";
        case CZIChecks::PlaneIndicesAreConsecutive: return "PlaneIndicesAreConsecutive";
        case CZIChecks::SubblocksHaveMindex: return "SubblocksHaveMindex";
        case CZIChecks::BasicMetadataValidation: return "BasicMetadataValidation";
        case CZIChecks::XmlMetadataSchemaValidation: return "XmlMetadataSchemaValidation";
        case CZIChecks::CCheckOverlappingScenesOnLayer0: return "CCheckOverlappingScenesOnLayer0";
        case CZIChecks::CheckSubBlockBitmapValid: return "CheckSubBlockBitmapValid";
        case CZIChecks::ConsistentMIndex: return "ConsistentMIndex";
        case CZIChecks::AttachmentDirectoryPositionsWithinRange: return "AttachmentDirectoryPositionsWithinRange";
        case CZIChecks::ApplianceMetadataTopographyItemValid: return "ApplianceMetadataTopographyItemValid";
        default: throw std::invalid_argument("No known conversion from CZIChecks to char");
    }
}

