#include "transform_roi_vr.h"

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "transform_implant_3d.h"

void TransformImplant3D::SyncBoneDensityCameraMatrix(
    const glm::mat4& rotate_mat, const glm::mat4& reorien_mat,
    const glm::mat4& view_mat) {
  SetRotateArcball(rotate_mat);
  set_reoren(reorien_mat);
  set_view(view_mat);
  UpdateMVP();
}
