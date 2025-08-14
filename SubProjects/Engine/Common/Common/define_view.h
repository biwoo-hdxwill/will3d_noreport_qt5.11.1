#pragma once

namespace common {
/*
View ID
*/
enum ViewTypeID {
  UNKNOWN = -1,
  MPR_AXIAL,
  MPR_SAGITTAL,
  MPR_CORONAL,
  MPR_3D,
  MPR_ZOOM3D,
  LIGHTBOX,
  PANO,
  PANO_ARCH,
  PANO_ORIENTATION,
  CROSS_SECTION,
  CROSS_SECTION_1,
  CROSS_SECTION_2,
  CROSS_SECTION_3,
  CROSS_SECTION_4,
  CROSS_SECTION_5,
  CROSS_SECTION_6,
  CROSS_SECTION_7,
  CROSS_SECTION_8,
  CROSS_SECTION_9,
  CROSS_SECTION_10,
  CROSS_SECTION_11,
  CROSS_SECTION_12,
  CROSS_SECTION_13,
  CROSS_SECTION_14,
  CROSS_SECTION_15,
  CROSS_SECTION_16,
  IMPLANT_SAGITTAL,
  IMPLANT_3D,
  IMPLANT_PREVIEW,
  IMPLANT_BONEDENSITY,
  ENDO,
  ENDO_MODIFY,
  ENDO_SAGITTAL,
  ENDO_SLICE,
  TMJ_FRONTAL_LEFT,
  TMJ_FRONTAL_RIGHT,
  TMJ_LATERAL_LEFT,
  TMJ_LATERAL_RIGHT,
  TMJ_ARCH,
  TMJ_ORIENTATION,
  SUPERIMPOSITION,
  CEPH,
  FACE_BEFORE,
  FACE_AFTER,
  FACE_PHOTO,
  FACE_SURFACE,
  PACS_MPR,
  PACS_3D
};

/*
        Values that represent common tool type onces.
        V_ : View tool
        M_ : Measure tool
*/
enum class CommonToolTypeOnce {
  V_RESET,
  V_FIT,
  V_INVERT,
  V_GRID,
  V_HIDE_TXT,
  V_HIDE_UI,
  M_HIDE_M,
  M_DEL_ALL,
  M_DEL_INCOMPLETION
};

/*
        Values that represent common tool type on offs.
        V_ : View tool
        M_ : Measure tool
        _L : Left Button
        _LR : Left + Right Button
*/
enum class CommonToolTypeOnOff {
  NONE,
  M_RULER,
  M_TAPELINE,
  M_TAPECURVE,
  M_ANGLE,
  M_PROFILE,
  M_AREALINE,
  M_ROI,
  M_RECTANGLE,
  M_CIRCLE,
  M_ARROW,
  M_LINE,
  M_FREEDRAW,
  M_NOTE,
  M_DEL,

  V_LIGHT,
  V_PAN,
  V_PAN_LR,
  V_ZOOM,
  V_ZOOM_R
};

} // end of namespace common
