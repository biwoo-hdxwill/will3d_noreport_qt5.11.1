#include "implant_data.h"

#include <qmath.h>

#if defined(__APPLE__)
#include <glm/gtx/transform.hpp>
#else
#include <GL/glm/gtx/transform.hpp>
#endif

ImplantData::ImplantData(const InitPack& init_pack)
  : id_(init_pack.implant_id),
  diameter_(init_pack.diameter),
  length_(init_pack.length),
	platform_diameter_(init_pack.platform_diameter),
	custom_apical_diameter_(init_pack.custom_apical_diameter),
	total_length_(init_pack.total_length),
  manufacturer_(init_pack.manufacturer),
  product_(init_pack.product),
  file_path_(init_pack.file_path),
	sub_category_(init_pack.sub_category),
  mesh_vertices_(init_pack.mesh_vertices),
  mesh_normals_(init_pack.mesh_normals),
  mesh_indices_(init_pack.mesh_indices),
  bounding_box_max_(init_pack.bounding_box_max),
  bounding_box_min_(init_pack.bounding_box_min),
  major_axis_(init_pack.major_axis) {

  if (init_pack.implant_id > 27) {
	rotate_in_vol_ = glm::rotate(float(M_PI), glm::vec3(1.0f, 0.0f, 0.0f));
  }
  rotate_in_pano_ =
	glm::rotate(-(float)M_PI_2, glm::vec3(1.0f, 0.0f, 0.0f)) * rotate_in_vol_;
}
