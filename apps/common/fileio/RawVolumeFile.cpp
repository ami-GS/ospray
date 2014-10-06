//
//                 INTEL CORPORATION PROPRIETARY INFORMATION
//
//    This software is supplied under the terms of a license agreement or
//    nondisclosure agreement with Intel Corporation and may not be copied
//    or disclosed except in accordance with the terms of that agreement.
//    Copyright (C) 2014 Intel Corporation. All Rights Reserved.
//

#include <stdio.h>
#include "apps/common/fileio/RawVolumeFile.h"

namespace ospray {

    OSPObjectCatalog RawVolumeFile::importVolume(Volume *pointer) {

        //! Look for the volume data file at the given path.
        FILE *file = fopen(filename.c_str(), "rb");  exitOnCondition(!file, "unable to open file '" + filename + "'");

        //! This loader operates on StructuredVolume objects.
        StructuredVolume *volume = dynamic_cast<StructuredVolume *>(pointer);  exitOnCondition(!volume, "unexpected volume type");

        //! Offset into the volume data file if any.
        size_t offset = volume->getParam1i("filename offset", 0);  fseek(file, offset, SEEK_SET);

        //! Create the equivalent ISPC volume container and allocate memory for voxel data.
        volume->createEquivalentISPC();

        //! Allocate memory for a single row of voxel data.
//      Data *buffer = new Data(volume->getDimensions().x, volume->getVoxelType(), NULL, 0);
        Data *buffer = new Data(volume->getDimensions().x, volume->getEnumForVoxelType(), NULL, 0);

        //! Copy voxel data into the volume a row at a time.
        for (size_t i=0 ; i < volume->getDimensions().y * volume->getDimensions().z ; i++) importVoxelRow(file, volume, buffer, i);

        //! Return an ObjectCatalog to allow introspection of the parameters.
        fclose(file);  return(new ObjectCatalog("volume", volume));

    }

    void RawVolumeFile::importVoxelRow(FILE *file, StructuredVolume *volume, Data *buffer, const size_t &index) {

        //! Indices of the voxel row in the volume.
        size_t j = index % volume->getDimensions().y;  size_t k = index / volume->getDimensions().y;

        //! Read the row of voxels from the volume file.
        size_t voxelsRead = fread(buffer->data, sizeOf(buffer->type), buffer->numItems, file);

        //! The end of the file may have been reached unexpectedly.
        exitOnCondition(voxelsRead != buffer->numItems, "end of volume file reached before read completed");

        //! Copy voxel data into the volume.
        volume->setRegion(buffer->data, vec3i(0, j, k), vec3i(buffer->numItems, 1, 1));

    }

} // namespace ospray
