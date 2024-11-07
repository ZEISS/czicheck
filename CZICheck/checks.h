// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

/// This enum defines the checks that can be performed by CZICheck.
#include <stdexcept>
enum class CZIChecks 
{
    /// It is checked whether all file-positions given in the subblock-directory are within the limits of the filesize.
    SubBlockDirectoryPositionsWithinRange,  

    /// Try to read all segments pointed to in the subblock-directory, and 
    /// check the segment header for validity, check whether the information about 
    /// the subblock in the subblock-directory and the subblock-segment itself is identical.
    SubBlockDirectorySegmentValid, 

    /// It is checked whether the coordinates of the subblocks are consistent. I.e. check whether all subblocks have 
    /// the same set of dimensions for their coordinate.
    ConsistentSubBlockCoordinates,  

    /// It is checked whether the are duplicate subblocks, i.e. subblocks with the same coordinates.
    DuplicateSubBlockCoordinates,   

    /// Check whether the document makes use of "B-index".
    BenabledDocument,

    /// Check whether all subblocks of a given channel index have the same pixeltype.
    SamePixeltypePerChannel,

    /// It is checked whether the plane-coordinates of all subblocks start at zero.
    PlanesIndicesStartAtZero,

    /// It is checked whether the plane-coordinates of all subblocks are consecutive; in other words, check for gaps.
    PlaneIndicesAreConsecutive,

    /// It is checked whether all subblocks have an 'M'-index.
    SubblocksHaveMindex,

    /// Some basic semantic checks of the XML-metadata, which is semantically validated with the content of the document.
    BasicMetadataValidation,

    /// Validate the XML-metadata against a schema. The availability of this
    /// checker is currently guarded with the configuration-option "CZICHECK_XERCESC_AVAILABLE".
    XmlMetadataSchemaValidation,

    /// It is checked whether subblocks within different scenes are overlapping (on pyramid-layer 0).
    CCheckOverlappingScenesOnLayer0,

    /// All subblocks are read AND their content is decoded and checked for consistency.
    CheckSubBlockBitmapValid,

    ConsistentMIndex,   ///< To be done, not yet implemented.

    AttachmentDirectoryPositionsWithinRange, ///< To be done, not yet implemented.

    /// The Applicance Metadata specified for TopographyDataItem(s) are valid
    ApplianceMetadataTopographyItemValid,
};

constexpr const char* CZIChecksToString(CZIChecks czi_check)
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



