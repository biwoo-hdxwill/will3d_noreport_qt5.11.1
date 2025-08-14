#pragma once
#include <qstring.h>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

namespace project {
enum class Purpose { LOAD, SAVE };

typedef struct _LoadVolInfo {
  float range_start = 0.0f;
  float range_end = 0.0f;
  int area_x = 0;
  int area_y = 0;
  int width = 0;
  int height = 0;
  float pixel_spacing = 0.0f;
} LoadVolInfo;

typedef struct _VolInfo {
  unsigned int width = 0;
  unsigned int height = 0;
  unsigned int depth = 0;
  unsigned short min = 0;
  unsigned short max = 0;
  float pixel_spacing = 0.0f;
  float slice_spacing = 0.0f;
  int window_center = 0;
  int window_width = 0;
  float slope = 0.0f;
  float intercept = 0.0f;
  int histo_size = 0;
  float slice_loc_maxilla = 0.0f;
  float slice_loc_teeth = 0.0f;
  float slice_loc_chin = 0.0f;
  float slice_loc_nose = 0.0f;
  int threshold_air = 0;
  int threshold_tissue = 0;
  int threshold_bone = 0;
} VolInfo;

typedef struct _Vec3Data {
  float x;
  float y;
  float z;
  void operator=(const _Vec3Data& arg) {
    x = arg.x;
    y = arg.y;
    z = arg.z;
  }
  _Vec3Data(const float& argx, const float& argy, const float& argz)
      : x(argx), y(argy), z(argz) {}
  _Vec3Data() : x(0.0f), y(0.0f), z(0.0f) {}
} Vec3Data;

typedef struct _MeasureInfo {
  int view_type = 0;
  unsigned int measure_id = 0;
  int measure_type = 0;
  int point_count = 0;
  unsigned int note_id = 0;
  unsigned int profile_id = 0;
  Vec3Data vp_center;
  Vec3Data vp_up;
  Vec3Data vp_back;
} MeasureInfo;

typedef struct _MeasureViewParams {
  int tab_type = 0;
  int view_type = 0;
  double center_x = 0, center_y = 0;
  float scale = 0.0f;
  float trans_x = 0.0f, trans_y = 0.0f;
  float pixel_spacing = 0.0f;
} MeasureViewInfo;

typedef struct _ViewInfo {
  float scale = 0.0f;
  float scale_scene_to_gl = 0.0f;
  float trans_x = 0.0f, trans_y = 0.0f;
} ViewInfo;

typedef struct _TMJRect {
  float left_width = 0.0f;
  float left_height = 0.0f;
  float right_width = 0.0f;
  float right_height = 0.0f;
} TMJRect;

typedef struct _TMJLateralParams {
  float left_interval = 0.0f;
  float left_thickness = 0.0f;
  float right_interval = 0.0f;
  float right_thickness = 0.0f;
} TMJLateralParms;

typedef struct _VTOSTOFlags {
  int is_set_isovalue = 0;
  int is_generate_head = 0;
  int is_make_tetra = 0;
  int is_fixed_isovalue_in_surgery = 0;
  int is_landmark = 0;
  int is_cut_face = 0;
  int is_do_mapping = 0;
  int is_calc_disp = 0;
  int is_make_surf = 0;
  int is_make_field = 0;
  int is_load_TRD = 0;
} VTOSTOFlags;

typedef struct _SurgeryCutOn {
  bool maxilla_on = false;
  bool mandible_on = false;
  bool chin_on = false;
} SurgeryCutOn;

typedef struct _STLTri {
  Vec3Data normal, v1, v2, v3, color;
  unsigned short cnt_attributes = 0;
  unsigned int color_value = 0;
} STLTri;

typedef struct _NerveParams {
  int color_r = 0, color_g = 0, color_b = 0;
  bool visible = false;
  double diameter_mm = 0.0;
  float radius = 0.0f;
  int id = 0;
} NerveParams;

typedef struct _ImplantDataParams {
  int id = 0;
  bool visible = false;
  float diameter = 0.0f, length = 0.0f;
  float platform_diameter = 0.0f, total_length = 0.0f;
  float custom_apical_diameter = 0.0f;
  QString product, manufacturer, path, sub_category;
  glm::vec3 pos_in_vol;
  glm::mat4 rot_in_vol;
  glm::mat4 trans_in_vol;
} ImplantDataParams;

typedef struct _ImplantResParams {
  QString memo;
  int selected_id = -1;
  std::vector<ImplantDataParams> datas;
} ImplantResParams;

enum class IOType { GENERAL, FILE, MPR, PANO, IMPLANT, TMJ, NONE };
}  // end of namespace project
