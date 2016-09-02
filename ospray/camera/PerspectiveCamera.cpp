// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "PerspectiveCamera.h"
#include <limits>
// ispc-side stuff
#include "PerspectiveCamera_ispc.h"

#ifdef _WIN32
#  define _USE_MATH_DEFINES
#  include <math.h> // M_PI
#endif

namespace ospray {

  PerspectiveCamera::PerspectiveCamera()
  {
    ispcEquivalent = ispc::PerspectiveCamera_create(this);
  }
  void PerspectiveCamera::commit()
  {
    Camera::commit();

    // ------------------------------------------------------------------
    // first, "parse" the additional expected parameters
    // ------------------------------------------------------------------
    fovy = getParamf("fovy", 60.f);
    aspect = getParamf("aspect", 1.f);
    apertureRadius = getParamf("apertureRadius", 0.f);
    focusDistance = getParamf("focusDistance", 1.f);

    // ------------------------------------------------------------------
    // now, update the local precomputed values
    // ------------------------------------------------------------------
    dir = normalize(dir);
    vec3f dir_du = normalize(cross(dir, up));
    vec3f dir_dv = cross(dir_du, dir);

    float imgPlane_size_y = 2.f*tanf(fovy/2.f*M_PI/180.);
    float imgPlane_size_x = imgPlane_size_y * aspect;

    dir_du *= imgPlane_size_x;
    dir_dv *= imgPlane_size_y;

    vec3f dir_00 = dir - .5f * dir_du - .5f * dir_dv;

    float scaledAperture = 0.f;
    // prescale to focal plane
    if (apertureRadius > 0.f) {
      dir_du *= focusDistance;
      dir_dv *= focusDistance;
      dir_00 *= focusDistance;
      scaledAperture = apertureRadius / imgPlane_size_x;
    }

    ispc::PerspectiveCamera_set(getIE(),
                                (const ispc::vec3f&)pos,
                                (const ispc::vec3f&)dir_00,
                                (const ispc::vec3f&)dir_du,
                                (const ispc::vec3f&)dir_dv,
                                scaledAperture,
                                aspect,
                                nearClip);
  }

  OSP_REGISTER_CAMERA(PerspectiveCamera,perspective);
  OSP_REGISTER_CAMERA(PerspectiveCamera,thinlens);

} // ::ospray







// Stereo, Work In Progress.

namespace ospray {
  
  PerspectiveStereoCamera::PerspectiveStereoCamera()
  {
    ispcEquivalent = ispc::PerspectiveStereoCamera_create(this);
  }
  void PerspectiveStereoCamera::commit()
  {
    Camera::commit();
    
    // ------------------------------------------------------------------
    // first, "parse" the additional expected parameters
    // ------------------------------------------------------------------
    fovy = getParamf("fovy", 60.f);
    aspect = getParamf("aspect", 1.f);
    apertureRadius = getParamf("apertureRadius", 0.f);
    focusDistance = getParamf("focusDistance", 1.f);
    ipdFactor = getParamf("ipdFactor", 30.f);
    
    cameraMode = getParam1i("cameraMode", 0);
    
    if (cameraMode == 0) {
      //aspect *= 2;// For double-wide buffer.
    }
    
    // ------------------------------------------------------------------
    // now, update the local precomputed values
    // ------------------------------------------------------------------
    dir = normalize(dir);
    vec3f dir_du = normalize(cross(dir, up));
    vec3f dir_dv = cross(dir_du, dir);
    
    //focusDistance = 4;
    
    //printf("focusDistance = %f\n", focusDistance);
    
    float imgPlane_size_y = 2.f*tanf(fovy/2.f*M_PI/180.) * focusDistance;
    float imgPlane_size_x = imgPlane_size_y * aspect;
    
    dir_du *= imgPlane_size_x;
    dir_dv *= imgPlane_size_y;
    
    vec3f dir_00 = dir - .5f * dir_du - .5f * dir_dv;
    
    ipdFactor = 30;// testing
    
    float ipdFactorCamera = focusDistance / ipdFactor;// 0.1f;
    float ipdFactorFilm = ipdFactorCamera / imgPlane_size_x;
    
    //ipdFactorCamera = 0;
    //ipdFactorFilm = 0;
    
    ipdFactorCamera = 20;// testing
    
    float scaledAperture = 0.f;
    // prescale to focal plane
    if (apertureRadius > 0.f) {
      dir_du *= focusDistance;
      dir_dv *= focusDistance;
      dir_00 *= focusDistance;
      scaledAperture = apertureRadius / imgPlane_size_x;
    }
    
    ispc::PerspectiveStereoCamera_set(getIE(),
                                (const ispc::vec3f&)pos,
                                (const ispc::vec3f&)dir_00,
                                (const ispc::vec3f&)dir_du,
                                (const ispc::vec3f&)dir_dv,
                                scaledAperture,
                                aspect,
                                nearClip,
                                cameraMode,
                                ipdFactorCamera,
                                ipdFactorFilm);
  }
  
  OSP_REGISTER_CAMERA(PerspectiveStereoCamera,perspectivestereo);
  OSP_REGISTER_CAMERA(PerspectiveStereoCamera,thinlensstereo);
  
} // ::ospray
