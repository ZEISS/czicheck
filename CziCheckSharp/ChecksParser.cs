// SPDX-FileCopyrightText: 2025 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

namespace CziCheckSharp;

public static class ChecksParser
{
    // Native checker kShortName values (used for the command line argument)
    private static readonly Dictionary<string, Checks> ShortNameMap = new(StringComparer.OrdinalIgnoreCase)
    {
        ["subblksegmentsinfile"] = Checks.HasValidSubBlockPositions,
        ["subblksegmentsvalid"] = Checks.HasValidSubBlockSegments,
        ["subblkdimconsistent"] = Checks.HasConsistentSubBlockDimensions,
        ["subblkcoordsunique"] = Checks.HasNoDuplicateSubBlockCoordinates,
        ["benabled"] = Checks.DoesNotUseBIndex,
        ["samepixeltypeperchannel"] = Checks.HasOnlyOnePixelTypePerChannel,
        ["planesstartindex"] = Checks.HasPlaneIndicesStartingAtZero,
        ["consecutiveplaneindices"] = Checks.HasConsecutivePlaneIndices,
        ["minallsubblks"] = Checks.AllSubblocksHaveMIndex,
        ["basicxmlmetadata"] = Checks.HasBasicallyValidMetadata,
        ["xmlmetadataschema"] = Checks.HasXmlSchemaValidMetadata,
        ["overlappingscenes"] = Checks.HasNoOverlappingScenesAtScale1,
        ["subblkbitmapvalid"] = Checks.HasValidSubBlockBitmaps,
        ["topographymetadata"] = Checks.HasValidApplianceMetadataTopography,
    };

    // C++ enum names (used in the json result)
    private static readonly Dictionary<string, Checks> CEnumNameMap = new(StringComparer.OrdinalIgnoreCase)
    {
        ["SubBlockDirectoryPositionsWithinRange"] = Checks.HasValidSubBlockPositions,
        ["SubBlockDirectorySegmentValid"] = Checks.HasValidSubBlockSegments,
        ["ConsistentSubBlockCoordinates"] = Checks.HasConsistentSubBlockDimensions,
        ["DuplicateSubBlockCoordinates"] = Checks.HasNoDuplicateSubBlockCoordinates,
        ["BenabledDocument"] = Checks.DoesNotUseBIndex,
        ["PlanesIndicesStartZero"] = Checks.HasPlaneIndicesStartingAtZero,
        ["PlaneIndicesAreConsecutive"] = Checks.HasConsecutivePlaneIndices,
        ["SubblocksHaveMindex"] = Checks.AllSubblocksHaveMIndex,
        ["BasicMetadataValidation"] = Checks.HasBasicallyValidMetadata,
        ["XmlMetadataSchemaValidation"] = Checks.HasXmlSchemaValidMetadata,
        ["CCheckOverlappingScenesOnLayer0"] = Checks.HasNoOverlappingScenesAtScale1,
        ["CheckSubBlockBitmapValid"] = Checks.HasValidSubBlockBitmaps,
#if FUTURE_CHECKS
        ["ConsistentMIndex"] = Checks.HasConsistentMIndices,
        ["AttachmentDirectoryPositionsWithinRange"] = Checks.HasValidAttachmentDirPositions,
#endif
        ["ApplianceMetadataTopographyItemValid"] = Checks.HasValidApplianceMetadataTopography,
    };

    public static bool TryParse(string? checksString, out Checks checks)
    {
        checks = Checks.None;
        if (string.IsNullOrWhiteSpace(checksString))
        {
            return false;
        }

        Checks result = Checks.None;
        string[] parts = checksString.Split(
            ',',
            StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
        foreach (string part in parts)
        {
            if (TryParseOneCheck(part, out Checks parsedCheck))
            {
                result |= parsedCheck;
            }
            else
            {
                return false;
            }
        }

        checks = result;
        return true;
    }

    private static bool TryParseOneCheck(string part, out Checks parsedCheck)
    {
        if (Enum.TryParse(part, ignoreCase: true, out parsedCheck))
        {
            return parsedCheck != Checks.None;
        }
        
        if (ShortNameMap.TryGetValue(part, out parsedCheck))
        {
            return true;
        }

        if (CEnumNameMap.TryGetValue(part, out parsedCheck))
        {
            return true;
        }

        parsedCheck = Checks.None;
        return false;
    }
}