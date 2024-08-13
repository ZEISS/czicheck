# Description of the checks performed

## Overview

The following table lists the available checkers and gives a short description of the checks performed by each of them.

| checker (shortname) | description |
|--|--|
|subblkdimconsistent|Test whether the coordinates of the subblocks are consistent. It is checked that all subblocks have the same set of dimensions for their coordinate.|
|subblksegmentsinfile|Test whether the subblock-segments listed in the subblock-directory are within the file. This checker does not validate the content of the subblock-segments, what is checked is only that the file-position (as reported in the subblock-directory) is within the limit of the file-size.  |
|subblksegmentsvalid|Read all subblocks from the file, and ensure their syntactical validity. Note that this check requires reading all data from disk, so it may be somewhat time consuming. |
|subblkcoordsunique|Test for subblocks with the same coordinate and position.  |
|benabled|Test whether the deprecated 'B'-dimension is used. |
|samepixeltypeperchannel|Test whether all subblocks of a given channel have the same pixeltype.|
|planesstartindex|Test whether the plane-coordinates start at '0'.|
|consecutiveplaneindices|Test whether the plane-coordinates are consecutive, in other words, check for gaps.|
|minallsubblks|Test whether all subblocks have an 'M'-index.|
|basicxmlmetadata|Same basic checks of the XML-metadata, which is semantically validated with the content of the file.|
|xmlmetadataschema|Validate the XML-metadata against the schema for it.|
|overlappingscenes|Test for subblocks within different scenes are overlapping (on pyramid-layer 0).|
|subblkbitmapvalid|Read all subblocks from the file, ensure their syntactical validity and decode the bitmap. Note that this check requires reading all data from disk and decoding, so it may be time consuming. |
|topographymetadata|Checks whether the given TopographyDataItems supplied in the Appliances metadata section of a czi image comply with the specification and the content of that czi image.|


## Details

### subblkdimconsistent

This checker is implemented in the file 'checkerConsistentCoordinates.cpp'.  
It is checked that all subbblocks have the same set of dimensions for their coordinate. The set of dimensions is determined for the first subblock, and we check whether all other subblocks contain the same set of dimensions.

###  subblksegmentsinfile

This checker is implemented in the file 'checkerSubBlkDirPositions.cpp'.  
Here the file-positions given in the subblock-directory are checked against the file-size. Note that only the file-position is checked, not the content of the subblock-segments.
Therefore the test is fast, and it will catch cases of truncated files. However - note that it will not catch cases where the subblock-segments are corrupted or where they are not readable. For a more thorough test 
the checker 'subblksegmentsvalid' should be used.

### subblksegmentsvalid

This checker is implemented in the file 'checkerSubBlkSegmentsValid.cpp'.  
Here all subblocks are read from the file, and their syntactical validity is checked. Note that this check requires reading all data from disk, so it may be somewhat time consuming.

### subblkcoordsunique

This checker is implemented in the file 'checkerDuplicateCoordinates.cpp'.  
It is tested whether subblocks exists which have a duplicate coordinate.

### benabled

This checker is implemented in the file 'checkerBenabled.cpp'.  
Usage of the dimension 'B' is deprecated, and this checker will test whether it is used.

### samepixeltypeperchannel

This checker is implemented in the file 'checkerSamePixelTypePerChannel.cpp'.  
It is tested whether all subblocks of a given channel have the same pixeltype.

### planesstartindex

This checker is implemented in the file 'checkerPlanesStartIndices.cpp'.  
It is tested whether the plane-coordinates start at index '0'.

### consecutiveplaneindices

This checker is implemented in the file 'checkerConsecutivePlaneIndices.cpp'.  
It is being checked whether the plane-coordinates are consecutive; in other words - check for gaps.

### minallsubblks

This checker is implemented in the file 'checkerMissingMindex.cpp'.  
It is checked whether all subblocks have an 'M'-index. It is recommended that all subblocks on pyramid-layer '0' have an 'M'-index, but this is not mandatory.
If subblocks without an M-index are found, this is reported as a warning.

### basicxmlmetadata

This checker is implemented in the file 'checkerXmlBasicMetadataValidation.cpp'.  
Some basic checks of the XML-metadata are performed, which is semantically validated with the content of the file.
* The content of the elements 'Metadata/Information/Image/Size_' are validated against the content of the subblock-directory. All dimensions which are
used in the subblock's coordinates should be listed (as 'Metadata/Information/Image/Size_') and their size should be given correctly.
* Under 'Metadata/Information/Image/Channels' there should the same number of 'Channel'-elements as there are channels in the subblock-directory.
* The pixel-type of a channel should be given correctly under 'Metadata/Information/Image/Channels/Channel[]/PixelType'.
* If the pixel-type is an integer pixel-type (or the pixel-type is unknown), the presence of the element 'ComponentBitCount' is checked for.
* In case of pixel-type and ComponentBitCount are given, the combination of both is checked for validity.

### xmlmetadataschema

This checker is implemented in the file 'checkerXmlMetadataXsdValidation.cpp'.  
The XML-metadata is validated against the schema for it.

### overlappingscenes

This checker is implemented in the file 'checkerOverlappingScenes.cpp'.  
It is tested whether subblocks within different scenes are overlapping (on pyramid-layer 0).

### subblkbitmapvalid

This checker is implemented in the file 'checkerSubBlkBitmapValid.cpp'.  
Here all subblocks are read from the file, and their syntactical validity is checked. The subblock's content is decoded, and the bitmap is checked for validity.
This check is more thorough than the check 'subblksegmentsvalid', as it also checks the content of the subblocks. The check is a strict superset of the check 'subblksegmentsvalid',
so it is not necessary to run both checks.
Note that this check requires reading all data from disk, and in case of compressed data it is decoded, so it may be time consuming.

### topographymetadata
This checker is implemented in the file 'checkerTopographyApplianceValidation.cpp'.  
It checks if an image contains a Topography section in its 'Appliances' metadata section.
If there is such a section, it checks if both 'Textures' and 'HeightMaps' are given within 'TopographyDataItem' containers in the Appliances section. Each of the entries in 'Textures' and 'HeighMaps' should specify a channel (via a 'StartC' information) and no other information. That other information is considered superfluous.