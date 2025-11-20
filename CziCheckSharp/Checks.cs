// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

/// <summary>
/// Flags enumeration specifying which CZI checks to run.
/// Maps to the CZICHECK_* bitmask constants in the native C API.
/// </summary>
/// <seealso href="https://github.com/ZEISS/czicheck/blob/main/documentation/description_of_checkers.md"/>
[Flags]
public enum Checks : ulong
{
    /// <summary>
    /// No checks selected.
    /// </summary>
    None = 0,

    /// <summary>
    /// SubBlock-Segment positions within file range (subblksegmentsinfile).
    /// Part of default set.
    /// </summary>
    HasValidSubBlockPositions = 0x0001UL,

    /// <summary>
    /// SubBlock-Segments in SubBlockDirectory are valid (SubBlockDirectorySegmentValid).
    /// Part of default set.
    /// </summary>
    HasValidSubBlockSegments = 0x0002UL,

    /// <summary>
    /// Check subblock's coordinates for 'consistent dimensions' (subblkdimconsistent).
    /// Part of default set.
    /// </summary>
    HasConsistentSubBlockDimensions = 0x0004UL,

    /// <summary>
    /// Check subblock's coordinates being unique (subblkcoordsunique).
    /// Part of default set.
    /// </summary>
    HasNoDuplicateSubBlockCoordinates = 0x0008UL,

    /// <summary>
    /// Check whether the document uses the deprecated 'B-index' (benabled).
    /// Part of default set.
    /// </summary>
    DoesNotUseBIndex = 0x0010UL,

    /// <summary>
    /// Check that the subblocks of a channel have the same pixel type (samepixeltypeperchannel).
    /// Part of default set.
    /// </summary>
    HasOnlyOnePixelTypePerChannel = 0x0020UL,

    /// <summary>
    /// Check that planes indices start at 0 (planesstartindex).
    /// Part of default set.
    /// </summary>
    HasPlaneIndicesStartingAtZero = 0x0040UL,

    /// <summary>
    /// Check that planes have consecutive indices (consecutiveplaneindices).
    /// Part of default set.
    /// </summary>
    HasConsecutivePlaneIndices = 0x0080UL,

    /// <summary>
    /// Check if all subblocks have the M index (minallsubblks).
    /// Part of default set.
    /// </summary>
    AllSubblocksHaveMIndex = 0x0100UL,

    /// <summary>
    /// Basic semantic checks of the XML-metadata (basicxmlmetadata).
    /// Part of default set.
    /// </summary>
    HasBasicallyValidMetadata = 0x0200UL,

    /// <summary>
    /// Validate the XML-metadata against XSD-schema (xmlmetadataschema).
    /// Opt-in check (expensive).
    /// </summary>
    HasXmlSchemaValidMetadata = 0x0400UL,

    /// <summary>
    /// Check if subblocks at pyramid-layer 0 of different scenes are overlapping (overlappingscenes).
    /// Part of default set.
    /// </summary>
    HasNoOverlappingScenesAtScale1 = 0x0800UL,

    /// <summary>
    /// SubBlock bitmap content validation (subblkbitmapvalid).
    /// Opt-in check (expensive).
    /// </summary>
    HasValidSubBlockBitmaps = 0x1000UL,

#if FUTURE_CHECKS
    /// <summary>
    /// Check for consistent M-Index usage (ConsistentMIndex).
    /// Not yet implemented.
    /// </summary>
    HasConsistentMIndices = 0x2000UL,

    /// <summary>
    /// Attachment directory positions within file range (AttachmentDirectoryPositionsWithinRange).
    /// Not yet implemented.
    /// </summary>
    HasValidAttachmentDirPositions = 0x4000UL,
#endif

    /// <summary>
    /// Basic semantic checks for TopographyDataItems (topographymetadata).
    /// Part of default set.
    /// </summary>
    HasValidApplianceMetadataTopography = 0x8000UL,

    /// <summary>
    /// The checks that are disabled by default (expensive operations).
    /// </summary>
    OptIn = HasXmlSchemaValidMetadata | HasValidSubBlockBitmaps,

    /// <summary>
    /// All available checks.
    /// </summary>
    All = 0xFFFFUL,

    /// <summary>
    /// Default set of checks (all checks that are not flagged as opt-in).
    /// </summary>
    Default = All & (~OptIn),
}