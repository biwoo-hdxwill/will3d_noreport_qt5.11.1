#include "pano_module.h"

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Math.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/image_2d.h"
#include "../../Resource/Resource/pano_resource.h"
#include "../../Resource/Resource/nerve_resource.h"
#include "../../Resource/Resource/cross_section_resource.h"
#include "../../Resource/Resource/sagittal_resource.h"
#include "../../Resource/Resource/implant_resource.h"

#include "../../Will3DEngine/renderer_manager.h"
#include "../../Renderer/implant_slice_renderer.h"
#include "../../Will3DEngine/implant_collision_renderer.h"

#ifdef _WIN64
#include <PanoPostProcessing.h>
#endif

using glm::vec3;
using namespace resource;

namespace {
  const glm::vec3 kInvAxisX(-1.0f, 1.0f, 1.0f);

  unsigned short ReconValue(const glm::vec3& pt_vol,
							unsigned short** buf_vol,
							const int& buf_width, const int& buf_height, const int& buf_depth) {
	const int x1 = static_cast<int>(pt_vol.x);
	const int y1 = static_cast<int>(pt_vol.y);
	const int z1 = static_cast<int>(pt_vol.z);
	const int x2 = std::min(x1 + 1, buf_width - 1);
	const int y2 = std::min(y1 + 1, buf_height - 1);
	const int z2 = std::min(z1 + 1, buf_depth - 1);
	const int Y1 = y1 * buf_width;
	const int Y2 = y2 * buf_width;

	return W3::trilerp(
	  buf_vol[z1][Y1 + x1], buf_vol[z1][Y1 + x2], buf_vol[z1][Y2 + x1], buf_vol[z1][Y2 + x2],
	  buf_vol[z2][Y1 + x1], buf_vol[z2][Y1 + x2], buf_vol[z2][Y2 + x1], buf_vol[z2][Y2 + x2],
	  pt_vol.x - x1, pt_vol.y - y1, pt_vol.z - z1);
  }

  bool IsCoordInRange(const glm::vec3& coordinate, const int& width, const int& height, const int& depth) {
	if ((coordinate.x >= 0 && coordinate.x < width)
		&& (coordinate.y >= 0 && coordinate.y < height)
		&& (coordinate.z >= 0 && coordinate.z < depth))
	  return true;
	else
	  return false;
  }

  bool IsCoordInMaskROI(const glm::vec3& coordinate, const glm::vec3& roi_start, const glm::vec3& roi_end) {
	if (((int)roi_start.x <= (int)coordinate.x) && ((int)roi_end.x >= (int)coordinate.x)
		&& ((int)roi_start.y <= (int)coordinate.y) && ((int)roi_end.y >= (int)coordinate.y)
		&& ((int)roi_start.z <= (int)coordinate.z) && ((int)roi_end.z >= (int)coordinate.z))
	  return true;
	else
	  return false;
  }

  bool GetImplantSliceRendererArgsInPanorama(const PanoResource& pano_resource,
											 const ImplantResource& implant_resource,
											 ImplantSliceRenderer::Arguments* args) {
	if (&pano_resource == nullptr)
	  return false;

	if (implant_resource.data().size() == 0)
	  return false;

	args->center_position_in_vol_gl = glm::vec3(0.0f, 0.0f, pano_resource.shifted_value() * 2.0f);
	args->slice_back_vector = glm::vec3(0.0f, 1.0f, 0.0f);
	args->slice_up_vector = glm::vec3(0.0f, 0.0f, 1.0f);
	args->img_width = pano_resource.GetPanoPlaneWidth();
	args->img_height = pano_resource.GetPanoPlaneHeight();

	if (args->img_width*args->img_height == 0)
	  return false;

	return true;
  }

  bool GetImplantSliceRendererArgsInSagittal(const CW3Image3D& vol,
											 const SagittalResource& sagittal_resource,
											 const ImplantResource& implant_resource,
											 ImplantSliceRenderer::Arguments* args) {
	if (&sagittal_resource == nullptr)
	  return false;

	if (implant_resource.data().size() == 0)
	  return false;

	const SagittalResource::Params& param = sagittal_resource.params();
	args->img_width = param.width;
	args->img_height = param.height;
	args->slice_back_vector = kInvAxisX * sagittal_resource.back_vector();
	args->slice_up_vector = kInvAxisX * sagittal_resource.up_vector();

	const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();
	const glm::vec3 pt_center_vol = glm::vec3((float)vol.width()*0.5f - 1.0f,
	  (float)vol.height()*0.5f - 1.0f,
											  (float)vol.depth()*0.5f - 1.0f);
	const glm::vec3 center_position_in_vol = sagittal_resource.center_position();
	const glm::vec3 center_position_in_vol_gl = kInvAxisX *
	  GLhelper::MapVolToWorldGL(center_position_in_vol, pt_center_vol, z_spacing);
	args->center_position_in_vol_gl = center_position_in_vol_gl;

	return true;
  }

  bool GetImplantSliceRendererArgsInCS(const CW3Image3D& vol,
									   const CrossSectionResource& cs_resource,
									   const ImplantResource& implant_resource,
									   int cross_id,
									   ImplantSliceRenderer::Arguments* args) {
	if (&cs_resource == nullptr)
	  return false;

	if (implant_resource.data().size() == 0)
	  return false;

	const std::unique_ptr<CrossSectionData>& cross_data = cs_resource.data().at(cross_id);
	CrossSectionResource::Params param = cs_resource.params();
	args->img_width = param.width;
	args->img_height = param.height;
	args->slice_back_vector = kInvAxisX * cross_data->back_vector();
	args->slice_up_vector = kInvAxisX * cross_data->up_vector();

	const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();
	const glm::vec3 pt_center_vol = glm::vec3((float)vol.width()*0.5f - 1.0f,
	  (float)vol.height()*0.5f - 1.0f,
											  (float)vol.depth()*0.5f - 1.0f);
	const glm::vec3 center_position_in_vol = cross_data->center_position_in_vol();
	const glm::vec3 center_position_in_vol_gl = kInvAxisX *
	  GLhelper::MapVolToWorldGL(center_position_in_vol, pt_center_vol, z_spacing);
	args->center_position_in_vol_gl = center_position_in_vol_gl;

	return true;
  }
}
/**=================================================================================================
public functions
*===============================================================================================**/
void PanoModule::ReconPanoPlane(const CW3Image3D& vol, PanoResource* pano_resource) {
  const CurveData& curve_data = pano_resource->GetCurrentCurveData();

  const int image_width = pano_resource->GetPanoPlaneWidth();
  const int image_height = pano_resource->GetPanoPlaneHeight();

  std::unique_ptr<Image2D> image(new Image2D(image_width, image_height, image::ImageFormat::GRAY16UI));

  const vec3 back_vector = pano_resource->back_vector();

  ushort *buf_plane = reinterpret_cast<ushort*>(image->Data());
  memset(buf_plane, 0, sizeof(ushort)*image_height*image_width);

  const std::vector<vec3>& curve_points = curve_data.points();

  ushort **buf_vol = vol.getData();

  const int vol_width = vol.width();
  const int vol_height = vol.height();
  const int vol_depth = vol.depth();
  const int intensity_min = vol.getMin();
  const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

  int num_thread = omp_get_max_threads();
  omp_set_num_threads(num_thread);

#pragma omp parallel for
  for (int v = 0; v < image_height; v++) {
	ushort *buf_temp = buf_plane + v * image_width;
	vec3 b_v = back_vector * float(v);
	for (int u = 0; u < image_width; u++) {
	  vec3 pt_vol = curve_points.at(u) + b_v;
	  pt_vol = W3::NormailzeCoordWithZspacing(pt_vol, z_spacing);

	  if (IsCoordInRange(pt_vol, vol_width, vol_height, vol_depth))
		*buf_temp++ = ReconValue(pt_vol, buf_vol, vol_width, vol_height, vol_depth);
	  else
		*buf_temp++ = intensity_min;
	}
  }

  image->SetPixelSpacing(vol.pixelSpacing());
  pano_resource->set_pano_image(image.release());


#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::ReconPanoPlane");
#endif 

}

void PanoModule::ReconNerveMaskPano(const CW3Image3D& vol, PanoResource* pano_resource,
									NerveResource* nerve_resource) {
  if (!nerve_resource->mask_roi().is_mask_filled()) {
	pano_resource->set_mask_nerve_image(nullptr);
	return;
  }

  const CurveData& curve_data = pano_resource->GetCurrentCurveData();

  int	mask_width = pano_resource->GetPanoPlaneWidth();
  int	mask_height = pano_resource->GetPanoPlaneHeight();

  std::unique_ptr<Image2D> mask(new Image2D(mask_width, mask_height, image::ImageFormat::RGBA32));
  uchar *buf_plane = reinterpret_cast<uchar*>(mask->Data());
  memset(buf_plane, 0, sizeof(uchar)*mask_width*mask_height * 4);

  const std::vector<vec3>& curve_points = curve_data.points();

  const int vol_width = vol.width();
  const int vol_height = vol.height();
  const int vol_depth = vol.depth();
  const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

  const NerveMaskROI& mask_roi = nerve_resource->mask_roi();
  const vec3 back_vector = pano_resource->back_vector();
  const int num_thread = omp_get_max_threads();
  omp_set_num_threads(num_thread);

#pragma omp parallel for
  for (int v = 0; v < mask_height; v++) {
	for (int u = 0; u < mask_width; u++) {

	  vec3 pt_vol = curve_points.at(u) + back_vector * float(v);
	  pt_vol = W3::NormailzeCoordWithZspacing(pt_vol, z_spacing);

	  int idxy = (int)pt_vol.y*vol_width + (int)pt_vol.x;
	  int idz = (int)pt_vol.z;
	  if (IsCoordInRange(pt_vol, vol_width, vol_height, vol_depth) &&
		  mask_roi.IsTrueBitMaskROI(idxy, idz)) {

		const auto& roi_dir = mask_roi.roi_direction();
		const auto& roi_start_pos = mask_roi.roi_start_pos();
		const auto& roi_end_pos = mask_roi.roi_end_pos();
		const auto& roi_radius = mask_roi.roi_radius();

		for (int i = 0; i < mask_roi.roi_list_size(); i++) {
		  const glm::vec3& roi_start = roi_start_pos[i];
		  const glm::vec3& roi_end = roi_end_pos[i];

		  if (IsCoordInMaskROI(pt_vol, roi_start, roi_end)) {
			const vec3 roi_center = (roi_start + roi_end)*0.5f;
			const vec3 roi_center_to_point = pt_vol - roi_center;

			float len_cross = glm::length(glm::cross(roi_center_to_point, roi_dir[i]));
			if (len_cross <= roi_radius[i]) {
			  int idx = (v*mask_width * 4) + u * 4;

			  const auto& roi_color = mask_roi.roi_color();
			  buf_plane[idx++] = (char)roi_color[i].red();
			  buf_plane[idx++] = (char)roi_color[i].green();
			  buf_plane[idx++] = (char)roi_color[i].blue();
			  buf_plane[idx] = (char)roi_color[i].alpha();

			  break;
			}
		  }
		}
	  }
	}
  }

  pano_resource->set_mask_nerve_image(mask.release());

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::ReconNerveMaskPano");
#endif 
}

void PanoModule::ReconNerveMaskPanoThickness(const CW3Image3D& vol, PanoResource* pano_resource,
											 NerveResource* nerve_resource) {
  if (!nerve_resource->mask_roi().is_mask_filled()) {
	pano_resource->set_mask_nerve_image(nullptr);
	return;
  }

  const CurveData& curve_data = pano_resource->GetCurrentCurveData();

  const int width = pano_resource->GetPanoPlaneWidth();
  const int height = pano_resource->GetPanoPlaneHeight();
  const int depth = pano_resource->thickness_value();

  std::unique_ptr<Image2D> mask(new Image2D(width, height, image::ImageFormat::RGBA32));

  const vec3 back_vector = pano_resource->back_vector();

  uchar *buf_plane = reinterpret_cast<uchar*>(mask->Data());
  memset(buf_plane, 0, sizeof(uchar)*width*height * 4);

  const std::vector<vec3>& curve_points = curve_data.points();
  const std::vector<vec3>& curve_up_vectors = curve_data.up_vectors();

  const float halfThickness = float(depth)*0.5f;

  const int vol_width = vol.width();
  const int vol_height = vol.height();
  const int vol_depth = vol.depth();

  const NerveMaskROI& mask_roi = nerve_resource->mask_roi();
  int num_thread = omp_get_max_threads();
  omp_set_num_threads(num_thread);

#pragma omp parallel for
  for (int v = 0; v < height; v++) {
	for (int u = 0; u < width; u++) {
	  int idx = ((v*width) + u) * 4;
	  for (int d = 0; d < depth; d++) {
		if (buf_plane[idx + 3])
		  continue;

		vec3 pt_vol = curve_points.at(u) + back_vector * float(v)
		  + curve_up_vectors.at(u)*(d - halfThickness);

		int idxy = (int)pt_vol.y*vol_width + (int)pt_vol.x;
		int idz = (int)pt_vol.z;
		if (IsCoordInRange(pt_vol, vol_width, vol_height, vol_depth) &&
			mask_roi.IsTrueBitMaskROI(idxy, idz)) {

		  const auto& roi_dir = mask_roi.roi_direction();
		  const auto& roi_start_pos = mask_roi.roi_start_pos();
		  const auto& roi_end_pos = mask_roi.roi_end_pos();
		  const auto& roi_radius = mask_roi.roi_radius();

		  for (int i = 0; i < mask_roi.roi_list_size(); i++) {
			glm::vec3 roi_start = roi_start_pos[i];
			glm::vec3 roi_end = roi_end_pos[i];

			if (IsCoordInMaskROI(pt_vol, roi_start, roi_end)) {
			  vec3 roi_center = (roi_start + roi_end)*0.5f;
			  vec3 roi_center_to_point = pt_vol - roi_center;

			  float len_cross = glm::length(glm::cross(roi_center_to_point, roi_dir[i]));
			  if (len_cross <= roi_radius[i]) {
				int iidx = idx;

				const auto& roi_color = mask_roi.roi_color();
				buf_plane[iidx++] = (char)roi_color[i].red();
				buf_plane[iidx++] = (char)roi_color[i].green();
				buf_plane[iidx++] = (char)roi_color[i].blue();
				buf_plane[iidx] = (char)roi_color[i].alpha();

				break;
			  }
			}
		  }
		}
	  }
	}
  }

  pano_resource->set_mask_nerve_image(mask.release());

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::ReconNerveMaskPanoThickness");
#endif 
}

void PanoModule::ReconNerveMaskPanoXray(const CW3Image3D& vol, PanoResource* pano_resource,
										NerveResource* nerve_resource) {
  if (!nerve_resource->mask_roi().is_mask_filled()) {
	pano_resource->set_mask_nerve_image(nullptr);
	return;
  }

  const CurveData& curve_data = pano_resource->curve_center_data();

  const int width = pano_resource->pano_3d_width();
  const int height = pano_resource->pano_3d_height();
  const int depth = pano_resource->pano_3d_depth();

  std::unique_ptr<Image2D> mask(new Image2D(width, height, image::ImageFormat::RGBA32));

  const vec3 back_vector = pano_resource->back_vector();

  uchar *buf_plane = reinterpret_cast<uchar*>(mask->Data());
  memset(buf_plane, 0, sizeof(uchar)*width*height * 4);

  const std::vector<vec3>& curve_points = curve_data.points();
  const std::vector<vec3>& curve_up_vectors = curve_data.up_vectors();

  const float halfThickness = float(depth)*0.5f;

  const int vol_width = vol.width();
  const int vol_height = vol.height();
  const int vol_depth = vol.depth();

  const NerveMaskROI& mask_roi = nerve_resource->mask_roi();
  int num_thread = omp_get_max_threads();
  omp_set_num_threads(num_thread);

#pragma omp parallel for
  for (int v = 0; v < height; v++) {
	for (int u = 0; u < width; u++) {
	  int idx = ((v*width) + u) * 4;
	  for (int d = 0; d < depth; d++) {
		if (buf_plane[idx + 3])
		  continue;

		vec3 pt_vol = curve_points.at(u) + back_vector * float(v)
		  + curve_up_vectors.at(u)*(d - halfThickness);

		int idxy = (int)pt_vol.y*vol_width + (int)pt_vol.x;
		int idz = (int)pt_vol.z;
		if (IsCoordInRange(pt_vol, vol_width, vol_height, vol_depth) &&
			mask_roi.IsTrueBitMaskROI(idxy, idz)) {

		  const auto& roi_dir = mask_roi.roi_direction();
		  const auto& roi_start_pos = mask_roi.roi_start_pos();
		  const auto& roi_end_pos = mask_roi.roi_end_pos();
		  const auto& roi_radius = mask_roi.roi_radius();

		  for (int i = 0; i < mask_roi.roi_list_size(); i++) {
			glm::vec3 roi_start = roi_start_pos[i];
			glm::vec3 roi_end = roi_end_pos[i];

			if (IsCoordInMaskROI(pt_vol, roi_start, roi_end)) {
			  vec3 roi_center = (roi_start + roi_end)*0.5f;
			  vec3 roi_center_to_point = pt_vol - roi_center;

			  float len_cross = glm::length(glm::cross(roi_center_to_point, roi_dir[i]));
			  if (len_cross <= roi_radius[i]) {
				int iidx = idx;

				const auto& roi_color = mask_roi.roi_color();
				buf_plane[iidx++] = (char)roi_color[i].red();
				buf_plane[iidx++] = (char)roi_color[i].green();
				buf_plane[iidx++] = (char)roi_color[i].blue();
				buf_plane[iidx] = (char)roi_color[i].alpha();

				break;
			  }
			}
		  }
		}
	  }
	}
  }

  pano_resource->set_mask_nerve_image(mask.release());

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::ReconNerveMaskPanoXray");
#endif 
}

void PanoModule::ReconPanoThickness(const CW3Image3D& vol, PanoResource* pano_resource) {
  const CurveData& curve_data = pano_resource->GetCurrentCurveData();
  const int width = pano_resource->GetPanoPlaneWidth();
  const int height = pano_resource->GetPanoPlaneHeight();
  const int depth = pano_resource->thickness_value();
  std::unique_ptr<Image2D> image(new Image2D(width, height, image::ImageFormat::GRAY16UI));

  const vec3& back_vector = pano_resource->back_vector();

  ushort *buf_plane = reinterpret_cast<ushort*>(image->Data());

  const std::vector<vec3>& curve_points = curve_data.points();
  const std::vector<vec3>& curve_up_vectors = curve_data.up_vectors();
  const float halfThickness = float(depth)*0.5f;
  ushort **buf_vol = vol.getData();

  const int vol_width = vol.width();
  const int vol_height = vol.height();
  const int vol_depth = vol.depth();
  const int intensity_min = vol.getMin();
  const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

  int num_thread = omp_get_max_threads();
  omp_set_num_threads(num_thread);

#pragma omp parallel for
  for (int v = 0; v < height; v++) {
	ushort *buf_temp = buf_plane + v * width;
	const vec3 b_v = back_vector * float(v);
	auto point_iter = curve_points.begin();
	auto upvec_iter = curve_up_vectors.begin();

	for (int u = 0; u < width; u++) {
	  float sum_intensity = 0.0f;
	  const vec3 pt_b_v = (*point_iter++) + b_v;
	  const vec3 up_vec = (*upvec_iter++);
	  for (int d = 0; d < depth; d++) {
		vec3 pt_vol = pt_b_v + up_vec * (d - halfThickness);
		pt_vol = W3::NormailzeCoordWithZspacing(pt_vol, z_spacing);

		if (IsCoordInRange(pt_vol, vol_width, vol_height, vol_depth))
		  sum_intensity += ReconValue(pt_vol, buf_vol, vol_width, vol_height, vol_depth);
		else
		  sum_intensity += intensity_min;
	  }

	  *buf_temp++ = sum_intensity / depth;
	}
  }

  image->SetPixelSpacing(vol.pixelSpacing());
  pano_resource->set_pano_image(image.release());

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::ReconPanoThickness");
#endif 
}
void PanoModule::ReconPanoXrayEnahancement(const CW3Image3D& vol, PanoResource* pano_resource) {
#ifdef _WIN64
  const CurveData& curve_data = pano_resource->curve_center_data();

  const int width = pano_resource->pano_3d_width();
  const int height = pano_resource->pano_3d_height();
  const int depth = pano_resource->pano_3d_depth();

  std::unique_ptr<Image2D> image(new Image2D(width, height, image::ImageFormat::GRAY16UI));

  if (width*height*depth == 0) {
	pano_resource->set_pano_image(image.release());
	return;
  }

  const vec3& back_vector = pano_resource->back_vector();

  ushort *buf_plane = reinterpret_cast<ushort*>(image->Data());

  const std::vector<vec3>& curve_points = curve_data.points();
  const std::vector<vec3>& curve_up_vectors = curve_data.up_vectors();
  const float halfThickness = float(depth)*0.5f;
  ushort **buf_vol = vol.getData();

  const int vol_width = vol.width();
  const int vol_height = vol.height();
  const int vol_depth = vol.depth();
  const int intensity_min = vol.getMin();
  const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

  int num_thread = omp_get_max_threads();
  omp_set_num_threads(num_thread);

#pragma omp parallel for
  for (int v = 0; v < height; v++) {
	ushort *buf_temp = buf_plane + v * width;
	const vec3 b_v = back_vector * float(v);
	auto point_iter = curve_points.begin();
	auto upvec_iter = curve_up_vectors.begin();
	for (int u = 0; u < width; u++) {
	  float sum_intensity = 0.0f;
	  const vec3 pt_b_v = (*point_iter++) + b_v;
	  const vec3 up_vec = (*upvec_iter++);
	  for (int d = 0; d < depth; d++) {
		vec3 pt_vol = pt_b_v + up_vec * (d - halfThickness);
		pt_vol = W3::NormailzeCoordWithZspacing(pt_vol, z_spacing);

		if (IsCoordInRange(pt_vol, vol_width, vol_height, vol_depth))
		  sum_intensity += ReconValue(pt_vol, buf_vol, vol_width, vol_height, vol_depth);
		else
		  sum_intensity += intensity_min;
	  }
	  *buf_temp++ = sum_intensity / depth;
	}
  }

  // YG Test buf_plane ���⼭ ó��
  CPanoPostProcessing filter;
  float sharpen_weight = 0.75f;
  float scale_GhostRemove = 0.5f;
  float ADF_iteration = 1.0f;


  // param tuning
  const float e = 0.00001f;
  if (halfThickness > 75.0f + e) {
	//sharpen_weight = 0.0012f*halfThickness + 0.66f;
	sharpen_weight = 0.0012f*halfThickness + 0.76f;
  }
  else if (halfThickness < 75.0f - e) {
	sharpen_weight = 0.01013f*halfThickness - 0.01013f;
	scale_GhostRemove = 0.0067f*halfThickness - 0.0067f;
  }


  if (1 <= 0.0f)
	sharpen_weight = 0.0f;

  //std::cout << "scale_GhostRemove : " << scale_GhostRemove << " , sharpen_weight : " << sharpen_weight << std::endl;

  const int SizeRecon = width * height;
  std::vector<unsigned short>   input_us(SizeRecon);

  unsigned short min = std::numeric_limits<unsigned short>::max();
  unsigned short max = std::numeric_limits<unsigned short>::min();
  int value = 0;
  for (int i = 0; i < SizeRecon; ++i) {
	value = buf_plane[i];
	input_us[i] = value;
	if (min > value)
	  min = value;

	if (max < value)
	  max = value;
  }

  //// Test
// FILE* file;
// fopen_s(&file, QString("%1_%2.raw").arg(width).arg(height).toStdString().c_str(),
//         "wb");
// fwrite(buf_plane, sizeof(unsigned short), width*height, file);
// fclose(file);

   //std::cout << "width : " << width << " , " << "height : " << height << std::endl;

  std::vector<unsigned short> out_us(SizeRecon);
  filter.doFiltering2(out_us, input_us, width, height, 0.6f, sharpen_weight, scale_GhostRemove, 255);

  ////// Test
  //FILE* file;
  //fopen_s(&file, QString("%1_%2.raw").arg(width).arg(height).toStdString().c_str(),
  //        "wb");
  //fwrite(&out_us[0], sizeof(unsigned short), width*height, file);
  //fclose(file);

	//std::cout << "window_center = " << vol.windowCenter() << std::endl;
	//std::cout << "window_width = " << vol.windowWidth() << std::endl;

	//// Test
	//unsigned short *temp1 = new unsigned short[SizeRecon];
	//for (int i = 0; i < SizeRecon; ++i)
	//   temp1[i] = out_us[i];
	//std::ofstream outfile1("inpout2.raw", std::ofstream::binary);
	//outfile1.write(reinterpret_cast<char*>(temp1), sizeof(unsigned short)*SizeRecon);
	//outfile1.close();

	//std::cout << "width : " << width << " , " << "height : " << height << std::endl;

  float window_min = (float)vol.windowCenter() - (float)vol.windowWidth() * 0.5f;
  float window_max = (float)vol.windowCenter() + (float)vol.windowWidth() * 0.5f;
  float window_width = (float)vol.windowWidth();
  float vol_max = (float)vol.getMax();
  unsigned short* out_us_data = &out_us[0];

  for (int i = 0; i < SizeRecon; ++i) {
	float value = ((float)(*out_us_data++) / 255.0f)*window_width + window_min;
	value = std::min(value, vol_max);
	value = std::max(value, 0.0f);

	buf_plane[i] = (ushort)value;
  }

  //// Test
  //unsigned short *temp0 = new unsigned short[SizeRecon];
  //for (int i = 0; i < SizeRecon; ++i)
  //   temp0[i] = buf_plane[i];
  //std::ofstream outfile0("inpout3.raw", std::ofstream::binary);
  //outfile0.write(reinterpret_cast<char*>(temp0), sizeof(unsigned short)*SizeRecon);
  //outfile0.close();

  //std::cout << "width : " << width << " , " << "height : " << height << std::endl;

  image->SetPixelSpacing(vol.pixelSpacing());
  pano_resource->set_pano_image(image.release());

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::ReconPanoXrayEnahancement");
#endif 
#endif
}


void PanoModule::ReconPano3D(const CW3Image3D & vol, PanoResource* pano_resource) {
  const CurveData& curve_data = pano_resource->curve_center_data();

  const int pano_vol_width = pano_resource->pano_3d_width();
  const int pano_vol_height = pano_resource->pano_3d_height();
  const int pano_vol_depth = pano_resource->pano_3d_depth();

  if (curve_data.GetCurveLength() < 2 ||
	  pano_vol_width * pano_vol_height * pano_vol_depth < 8)
  {
	  pano_resource->set_pano_vol(nullptr);
	  return;
  }

  std::unique_ptr<CW3Image3D> pano_vol;
  pano_vol.reset(new CW3Image3D(pano_vol_width, pano_vol_height, pano_vol_depth));
  pano_vol->setPixelSpacing(vol.pixelSpacing());
  pano_vol->setSliceSpacing(vol.sliceSpacing());
  pano_vol->setThreshold(vol.getAirTissueThreshold(), vol.getTissueBoneThreshold(),
						 vol.getBoneTeethThreshold());
  pano_vol->setMinMax(vol.getMin(), vol.getMax());
  pano_vol->setWindowing(vol.windowCenter(), vol.windowWidth());

  const vec3 back_vector = pano_resource->back_vector();

  ushort **buf_pano_vol = pano_vol->getData();

  const std::vector<vec3>& curve_points = curve_data.points();
  const std::vector<vec3>& curve_up_vectors = curve_data.up_vectors();
  const float halfThickness = float(pano_vol_depth)*0.5f;
  ushort **buf_vol = vol.getData();

  const int vol_width = vol.width();
  const int vol_height = vol.height();
  const int vol_depth = vol.depth();
  const int intensity_min = vol.getMin();
  const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

  int num_thread = omp_get_max_threads();
  omp_set_num_threads(num_thread);

#pragma omp parallel for
  for (int d = 0; d < pano_vol_depth; d++) {
	ushort *buf_temp = buf_pano_vol[d];
	for (int v = 0; v < pano_vol_height; v++) {
	  const vec3 b_v = back_vector * float(v);
	  auto point_iter = curve_points.begin();
	  auto upvec_iter = curve_up_vectors.begin();

	  for (int u = 0; u < pano_vol_width; u++) {
		vec3 pt_vol = *point_iter++ + b_v + *upvec_iter++*(d - halfThickness);
		pt_vol = W3::NormailzeCoordWithZspacing(pt_vol, z_spacing);

		if (IsCoordInRange(pt_vol, vol_width, vol_height, vol_depth))
		  *buf_temp++ = ReconValue(pt_vol, buf_vol, vol_width, vol_height, vol_depth);
		else
		  *buf_temp++ = intensity_min;
	  }
	}
  }

  pano_resource->set_pano_vol(pano_vol.release());

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::ReconPano3D");
#endif 
}

void PanoModule::ReconPanoImplantMask(const CW3Image3D & vol, PanoResource* pano_resource,
									  ImplantResource* implant_resource, float view_scale) {
  pano_resource->set_mask_implant_image(nullptr);

  ImplantSliceRenderer::Arguments args;
  bool result = GetImplantSliceRendererArgsInPanorama(*pano_resource, *implant_resource, &args);

  if (!result)
	return;

  args.scale = std::min(sqrt(view_scale), 3.0f);
  args.scale = std::max(args.scale, 1.0f);

  ImplantSliceRenderer& implant_renderer = RendererManager::GetInstance().renderer_implant();
  resource::Image2D* result_img;
  implant_renderer.RenderInPano(args, result_img);
  pano_resource->set_mask_implant_image(result_img);

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::ReconPanoImplantMask");
#endif 
}

void PanoModule::MapVolToPanoPlane(const PanoResource & pano_resource,
								   const std::vector<glm::vec3>& src_pts_vol,
								   std::vector<QPointF>& dst_pts_pano_plane) {
  dst_pts_pano_plane.clear();
  dst_pts_pano_plane.reserve(src_pts_vol.size());

  for (const auto& pt_vol : src_pts_vol)
	dst_pts_pano_plane.push_back(MapVolToPanoPlane(pano_resource, pt_vol));
}

QPointF PanoModule::MapVolToPanoPlane(const PanoResource& pano_resource, const glm::vec3& pt_vol) {
  const CurveData& curve_data = pano_resource.GetCurrentCurveData();
  const std::vector<glm::vec3>& points = curve_data.points();

  if (points.empty())
	return QPointF();

  float min_dist = std::numeric_limits<float>::max();
  int min_i = 0;
  QPointF pt_pano_plane;
  const glm::vec3 back_vector = glm::normalize(pano_resource.back_vector());
  for (int i = 0; i < points.size(); i++) {
	float dist = glm::length(pt_vol - points[i]);
	if (min_dist > dist) {
	  min_dist = dist;
	  min_i = i;
	}
  }

  int plane_y = (int)glm::dot(pt_vol - points[min_i], back_vector);
  pt_pano_plane = QPointF(min_i, plane_y);

  bool is_out_range = (min_i == 0 || min_i == points.size() - 1);
  if (is_out_range) {
	float sign = (min_i == 0) ? -1.0f : 1.0f;

	glm::vec3 pt_output_vol = MapPanoPlaneToVol(pano_resource, pt_pano_plane);
	glm::vec3 pt_err_vec = pt_output_vol - pt_vol;
	float error = glm::length(pt_err_vec);
	const float kErrorThreshold = 3.0f;
	if (kErrorThreshold < error) {
	  const std::vector<glm::vec3>& up_vectors = curve_data.up_vectors();
	  glm::vec3 dir;
	  if (sign > 0.0f)
		dir = glm::normalize(glm::cross(up_vectors[min_i], back_vector));
	  else
		dir = glm::normalize(glm::cross(back_vector, up_vectors[min_i]));

	  float dot = glm::dot(dir, pt_err_vec);
	  glm::vec3 point = dir * dot + points[min_i];
	  return QPointF(min_i + dot * sign,
					 glm::dot(pt_vol - point, back_vector));
	}
  }

  return pt_pano_plane;
}

QPointF PanoModule::MapVolToPanoPlane_Sagittal(const PanoResource & pano_resource, const glm::vec3 & pt_vol) {
  const CurveData& curve_data = pano_resource.GetCurrentCurveData();
  const std::vector<glm::vec3>& points = curve_data.points();

  if (points.empty())
	return QPointF();

  float min_dist = std::numeric_limits<float>::max();
  int min_i = 0;
  QPointF pt_pano_plane;
  const glm::vec3 back_vector = glm::normalize(pano_resource.back_vector());
  for (int i = 0; i < points.size(); i++) {
	float dist = glm::length(pt_vol - points[i]);
	if (min_dist > dist) {
	  min_dist = dist;
	  min_i = i;
	}
  }

  int plane_y = (int)glm::dot(pt_vol - points[min_i], back_vector);
  pt_pano_plane = QPointF(min_i, plane_y);
  return pt_pano_plane;
}

void PanoModule::MapPanoPlaneToVol(const PanoResource& pano_resource,
								   const std::vector<QPointF>& src_pts_pano_plane,
								   std::vector<glm::vec3>& dst_pts_vol,
								   bool is_print_log) {
  dst_pts_vol.clear();
  dst_pts_vol.reserve(src_pts_pano_plane.size());

  for (const auto& pt_plane : src_pts_pano_plane)
	dst_pts_vol.push_back(MapPanoPlaneToVol(pano_resource, pt_plane, is_print_log));
}

glm::vec3 PanoModule::MapPanoPlaneToVol(const PanoResource& pano_resource, const QPointF& pt_pano_plane,
										bool is_print_log) {
  const CurveData& curve_data = pano_resource.GetCurrentCurveData();
  const glm::vec3 y_offset = glm::normalize(pano_resource.back_vector())*(float)pt_pano_plane.y();
  const std::vector<glm::vec3>& points = curve_data.points();

  int x = (int)pt_pano_plane.x();
  int y = (int)pt_pano_plane.y();
  if (x >= 0 && x < pano_resource.GetPanoPlaneWidth() - 1 && y >= 0 && y < pano_resource.GetPanoPlaneHeight()) {
	float x_offset = pt_pano_plane.x() - x;
	return (points[x + 1] - points[x])*x_offset + points[x] + y_offset;
  }
  else if (x == pano_resource.GetPanoPlaneWidth() - 1) {
	float x_offset = pt_pano_plane.x() - x;
	return (points[x - 1] - points[x])*x_offset + points[x] + y_offset;
  }
  else {
	if (is_print_log) {
	  auto logger = common::Logger::instance();
	  logger->Print(common::LogType::WRN, "PanoModule::MapPanoPlaneToVol: index out range");
	}
	return panorama::kInvalidVolPt;
  }
}

QPointF PanoModule::MapVolToCrossSectionPlane(const CrossSectionResource& cross_resource, int cross_id,
											  const glm::vec3& pt_vol) {
  const std::map<int, std::unique_ptr<CrossSectionData>>& cross_data_list = cross_resource.data();

  const auto& cross_iter = cross_data_list.find(cross_id);
  if (cross_iter == cross_data_list.end() || !cross_iter->second->is_init()) {
	common::Logger::instance()->Print(common::LogType::WRN,
									  "PanoModule::MapCrossSectionPlaneToVol: invalid id.");
	return QPointF();
  }

  const auto& curve_data = cross_iter->second;

  if (!curve_data->is_init())
	return QPointF();

  const glm::vec3& center_pos = curve_data->center_position_in_vol();
  const glm::vec3& right_vector = curve_data->right_vector();
  const glm::vec3& back_vector = curve_data->back_vector();

  const glm::vec3& disp = pt_vol - center_pos;
  double disp_x = (double)glm::dot(right_vector, disp);
  double disp_y = (double)glm::dot(back_vector, disp);
  QPointF plane_center = QPointF((double)(cross_resource.params().width / 2),
	(double)(cross_resource.params().height / 2));

  return QPointF(plane_center.x() + disp_x,
				 plane_center.y() + disp_y);

}

void PanoModule::MapCrossSectionPlaneToVol(const CrossSectionResource& cross_resource, int cross_id,
										   const std::vector<QPointF>& src_pts_cross_plane,
										   std::vector<glm::vec3>& dst_pts_vol,
										   bool is_print_log) {
  dst_pts_vol.clear();
  dst_pts_vol.reserve(src_pts_cross_plane.size());

  for (const auto& pt_plane : src_pts_cross_plane)
	dst_pts_vol.push_back(MapCrossSectionPlaneToVol(cross_resource, cross_id,
						  pt_plane, is_print_log));
}
glm::vec3 PanoModule::MapCrossSectionPlaneToVol(const CrossSectionResource& cross_resource, int cross_id,
												const QPointF& pt_cross_section,
												bool is_print_log) {
  const std::map<int, std::unique_ptr<CrossSectionData>>& cross_data_list = cross_resource.data();

  const auto& cross_iter = cross_data_list.find(cross_id);
  if (is_print_log && cross_iter == cross_data_list.end()) {
	common::Logger::instance()->Print(common::LogType::WRN,
									  "PanoModule::MapCrossSectionPlaneToVol: invalid id.");
	return panorama::kInvalidVolPt;
  }


  const auto& curve_data = cross_iter->second;

  if (!curve_data->is_init())
	return panorama::kInvalidVolPt;

  const int image_height = cross_resource.params().height;
  const int image_width = cross_resource.params().width;
  const int x = pt_cross_section.x();
  const int y = pt_cross_section.y();

  if (x >= 0 && x < image_width && y >= 0 && y < image_height) {
	const glm::vec3& right_vector = curve_data->right_vector();
	const glm::vec3& back_vector = curve_data->back_vector();
	return curve_data->center_position_in_vol() +
	  (float)(x - image_width / 2) * right_vector +
	  (float)(y - image_height / 2) * back_vector;
  }
  else {
	if (is_print_log) {
	  common::Logger::instance()->Print(common::LogType::WRN,
										"PanoModule::MapCrossSectionPlaneToVol: index out range");
	}
	return panorama::kInvalidVolPt;
  }
}

QPointF PanoModule::MapVolToSagittalPlane(const SagittalResource & sagittal_resource,
										  const glm::vec3& pt_vol) {
  if (!sagittal_resource.is_valid())
	return QPointF();

  const glm::vec3& center_pos = sagittal_resource.center_position();
  const glm::vec3& right_vector = sagittal_resource.right_vector();
  const glm::vec3& back_vector = sagittal_resource.back_vector();

  const glm::vec3& disp = pt_vol - center_pos;
  double disp_x = (double)glm::dot(right_vector, disp);
  double disp_y = (double)glm::dot(back_vector, disp);
  QPointF plane_center = QPointF((double)(sagittal_resource.params().width / 2),
	(double)(sagittal_resource.params().height / 2));

  return QPointF(plane_center.x() + disp_x,
				 plane_center.y() + disp_y);
}

void PanoModule::MapSagittalPlaneToVol(const SagittalResource & sagittal_resource,
									   const std::vector<QPointF>& src_pts_sagittal_plane,
									   std::vector<glm::vec3>& dst_pts_vol,
									   bool is_print_log) {
  dst_pts_vol.clear();
  dst_pts_vol.reserve(src_pts_sagittal_plane.size());

  for (const auto& pt_pano3d : src_pts_sagittal_plane) {
	dst_pts_vol.push_back(MapSagittalPlaneToVol(sagittal_resource, pt_pano3d, false));
  }
}

glm::vec3 PanoModule::MapSagittalPlaneToVol(const SagittalResource & sagittal_resource,
											const QPointF & pt_sagittal, bool is_print_log) {
  if (!sagittal_resource.is_valid())
	return panorama::kInvalidVolPt;

  const int image_height = sagittal_resource.params().height;
  const int image_width = sagittal_resource.params().width;
  const int x = pt_sagittal.x();
  const int y = pt_sagittal.y();

  if (x >= 0 && x < image_width && y >= 0 && y < image_height) {
	const glm::vec3& right_vector = sagittal_resource.right_vector();
	const glm::vec3& back_vector = sagittal_resource.back_vector();
	return sagittal_resource.center_position() +
	  (float)(x - image_width / 2) * right_vector +
	  (float)(y - image_height / 2) * back_vector;
  }
  else {
	if (is_print_log) {
	  common::Logger::instance()->Print(common::LogType::WRN,
										"PanoModule::MapSagittalPlaneToVol: index out range");
	}
	return panorama::kInvalidVolPt;
  }
}

void PanoModule::MapPano3DToVol(const PanoResource& pano_resource,
								const std::vector<glm::vec3>& src_pts_pano3d,
								std::vector<glm::vec3>& dst_pts_vol) {
  dst_pts_vol.clear();
  dst_pts_vol.reserve(src_pts_pano3d.size());

  for (const auto& pt_pano3d : src_pts_pano3d) {
	dst_pts_vol.push_back(MapPano3DToVol(pano_resource, pt_pano3d));
  }
}
glm::vec3 PanoModule::MapPano3DToVol(const PanoResource& pano_resource, const glm::vec3& pt_pano3d) {
  const CurveData& curve_data = pano_resource.curve_center_data();
  const std::vector<glm::vec3>& curve_points = curve_data.points();

  const int pt_x_pano3d = (int)pt_pano3d.x;
  const glm::vec3 back_vector = glm::normalize(pano_resource.back_vector());
  const std::vector<glm::vec3>& up_vectors = curve_data.up_vectors();

  if (pt_x_pano3d >= 0 && pt_x_pano3d < pano_resource.pano_3d_width() - 1) {
	float offset = pt_pano3d.x - pt_x_pano3d;
	const glm::vec3 up_vector = glm::normalize(up_vectors[pt_x_pano3d] +
	  (up_vectors[pt_x_pano3d + 1] - up_vectors[pt_x_pano3d])*offset);
	const glm::vec3 position = curve_points[pt_x_pano3d] +
	  (curve_points[pt_x_pano3d + 1] - curve_points[pt_x_pano3d])*offset;

	return position + back_vector * pt_pano3d.y + up_vector * pt_pano3d.z;
  }
  else if (pt_x_pano3d == pano_resource.pano_3d_width() - 1) {
	float offset = pt_pano3d.x - pt_x_pano3d;
	const glm::vec3 up_vector = glm::normalize(up_vectors[pt_x_pano3d] +
	  (up_vectors[pt_x_pano3d - 1] - up_vectors[pt_x_pano3d])*offset);
	const glm::vec3 position = curve_points[pt_x_pano3d] +
	  (curve_points[pt_x_pano3d - 1] - curve_points[pt_x_pano3d])*offset;

	return position + back_vector * pt_pano3d.y + up_vector * pt_pano3d.z;
  }
  else if (pt_x_pano3d < 0) {
	glm::vec3 inv_right_vec = glm::normalize(glm::cross(up_vectors[0], back_vector));
	glm::vec3 position = inv_right_vec * (-pt_pano3d.x) + curve_points[0];

	return position + back_vector * pt_pano3d.y + up_vectors[0] * pt_pano3d.z;
  }
  else if (pt_x_pano3d > pano_resource.pano_3d_width() - 1) {
	int idx = pano_resource.pano_3d_width() - 1;
	glm::vec3 right_vec = glm::normalize(glm::cross(back_vector, up_vectors[idx]));
	glm::vec3 position = right_vec * (pt_pano3d.x - (float)idx) + curve_points[idx];

	return position + back_vector * pt_pano3d.y + up_vectors[idx] * pt_pano3d.z;
  }
  else {
	assert(false);
	return glm::vec3();
  }
}

void PanoModule::MapVolToPano3D(const PanoResource & pano_resource,
								const std::vector<glm::vec3>& src_pts_vol,
								std::vector<glm::vec3>& dst_pts_pano3d) {
  dst_pts_pano3d.clear();
  dst_pts_pano3d.reserve(src_pts_vol.size());

  for (const auto& pt_vol : src_pts_vol)
	dst_pts_pano3d.push_back(MapVolToPano3D(pano_resource, pt_vol));
}

glm::vec3 PanoModule::MapVolToPano3D(const PanoResource& pano_resource, const glm::vec3& pt_vol) {
  const CurveData& curve_data = pano_resource.curve_center_data();
  const std::vector<glm::vec3>& points = curve_data.points();
  const std::vector<glm::vec3>& up_vectors = curve_data.up_vectors();
  const glm::vec3 back_vector = glm::normalize(pano_resource.back_vector());

  if (points.size() == 0)
	return glm::vec3();

  glm::vec3 pt_pano3d;
  float min_dist = std::numeric_limits<float>::max();
  int min_i = 0;
  for (int i = 0; i < points.size(); i++) {
	float dist = glm::length(pt_vol - points[i]);

	if (min_dist > dist) {
	  min_dist = dist;
	  min_i = i;
	}
  }

  bool is_out_range = (min_i == 0 ||
					   min_i == points.size() - 1);
  if (is_out_range) {
	float sign;
	sign = (min_i == 0) ? -1.0f : 1.0f;

	glm::vec3 pt_pano_3d_ = vec3((float)min_i,
								 glm::dot(pt_vol - points[min_i], back_vector),
								 glm::dot(pt_vol - points[min_i], up_vectors[min_i]));
	glm::vec3 pt_output_vol = MapPano3DToVol(pano_resource, pt_pano_3d_);
	glm::vec3 pt_err_vec = pt_output_vol - pt_vol;
	float error = glm::length(pt_err_vec);
	const float kErrorThreshold = 3.0f;
	if (kErrorThreshold < error) {
	  glm::vec3 dir;

	  if (sign > 0.0f)
		dir = glm::normalize(glm::cross(up_vectors[min_i], back_vector));
	  else
		dir = glm::normalize(glm::cross(back_vector, up_vectors[min_i]));

	  float dot = glm::dot(dir, pt_err_vec);
	  glm::vec3 point = dir * dot + points[min_i];
	  return vec3(min_i + dot * sign,
				  glm::dot(pt_vol - point, back_vector),
				  glm::dot(pt_vol - point, up_vectors[min_i]));
	}
  }

  const int kSamplingCnt = 10;
  float samp_begin = std::max((float)min_i - 1.0f, 0.0f);
  float samp_end = std::min((float)min_i + 1.0f, (float)points.size() - 1.0f);
  float samp_step = (float)(samp_end - samp_begin) / (float)kSamplingCnt;
  float dot_max = std::numeric_limits<float>::min();
  float pick_idx = 0.0f;

  for (int i = 0; i < kSamplingCnt; i++) {
	float samp_idx = samp_begin + samp_step * i;
	glm::vec3 vol_vec = glm::normalize(pt_vol - curve_data.GetInterpolatedPoint(samp_idx));
	glm::vec3 up_vec = curve_data.GetInterpolatedUpvector(samp_idx);

	float dot = abs(glm::dot(vol_vec, up_vec));
	if (dot_max < dot) {
	  dot_max = dot;
	  pick_idx = samp_idx;
	}
  }

  glm::vec3 pick_curve_point = curve_data.GetInterpolatedPoint(pick_idx);
  return vec3(pick_idx,
			  glm::dot(pt_vol - pick_curve_point, back_vector),
			  glm::dot(pt_vol - pick_curve_point, curve_data.GetInterpolatedUpvector(pick_idx)));
}

void PanoModule::GetPanoDirection(const PanoResource & pano_resource, const QPointF& pt_pano_plane,
								  glm::vec3& pt_prev, glm::vec3& pt_next) {
  const CurveData& curve_data = pano_resource.GetCurrentCurveData();
  const glm::vec3 y_offset = glm::normalize(pano_resource.back_vector())*(float)pt_pano_plane.y();
  const std::vector<glm::vec3>& points = curve_data.points();
  int pano_index = static_cast<int>(pt_pano_plane.x());
  if (pano_index == 0) {
	pt_prev = points[pano_index];
	pt_next = points[pano_index + 1];
  }
  else if (pano_index == (int)points.size() - 1) {
	pt_prev = points[pano_index - 1];
	pt_next = points[pano_index];
  }
  else {
	pt_prev = points[pano_index - 1];
	pt_next = points[pano_index + 1];
  }

  pt_prev += y_offset;
  pt_next += y_offset;
}
void PanoModule::CheckCollideNerve(ImplantResource * implant_resource, const glm::mat4& proj_view_mat) {
  ImplantCollisionRenderer& implant_renderer = RendererManager::GetInstance().implant_collision_renderer();
  std::vector<int> collided_ids;
  implant_renderer.CheckCollide(proj_view_mat, &collided_ids, false);

  implant_resource->SetCollideIds(collided_ids);
}
void PanoModule::CheckCollideImplant(ImplantResource * implant_resource, const glm::mat4& proj_view_mat) {
  ImplantCollisionRenderer& implant_renderer = RendererManager::GetInstance().implant_collision_renderer();
  std::vector<int> collided_ids;
  implant_renderer.CheckCollide(proj_view_mat, &collided_ids, true);

  implant_resource->SetCollideIds(collided_ids);
}

void PanoModule::HoveredImplantInPanoPlane(const PanoResource & pano_resource,
										   const ImplantResource & implant_resource,
										   const QPointF & pt_in_pano_plane, int* implant_id) {
  ImplantSliceRenderer::Arguments args;
  bool result = GetImplantSliceRendererArgsInPanorama(pano_resource, implant_resource, &args);

  if (!result)
	return;


  ImplantSliceRenderer& implant_renderer = RendererManager::GetInstance().renderer_implant();
  implant_renderer.PickInPano(args, pt_in_pano_plane, implant_id);

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::HoveredImplantInPanoPlane");
#endif 
}

void PanoModule::HoveredImplantInSagittalPlane(const CW3Image3D& vol, const SagittalResource& sagittal_resource,
											   const ImplantResource& implant_resource,
											   const QPointF& pt_in_sagittal_plane, int* implant_id) {
  ImplantSliceRenderer::Arguments args;
  bool result = GetImplantSliceRendererArgsInSagittal(vol, sagittal_resource, implant_resource,
													  &args);
  if (!result)
	return;

  ImplantSliceRenderer& implant_renderer = RendererManager::GetInstance().renderer_implant();
  implant_renderer.PickInVol(args, pt_in_sagittal_plane, implant_id);

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::HoveredImplantInSagittalPlane");
#endif 
}

void PanoModule::HoveredImplantInCSPlane(const CW3Image3D & vol, const CrossSectionResource & cs_resource,
										 const ImplantResource& implant_resource,
										 int cross_id, const QPointF & pt_in_cs_plane,
										 int& implant_id, QPointF& implant_pos) {
  ImplantSliceRenderer::Arguments args;
  bool result = GetImplantSliceRendererArgsInCS(vol, cs_resource, implant_resource, cross_id, &args);

  if (!result)
	return;

  ImplantSliceRenderer& implant_renderer = RendererManager::GetInstance().renderer_implant();
  implant_renderer.PickInVol(args, pt_in_cs_plane, &implant_id);

  if (implant_id > 0) {
	const auto& implant_data = implant_resource.data().at(implant_id);
	glm::vec3 implant_pt_vol = implant_data->position_in_vol();

	implant_pos = MapVolToCrossSectionPlane(cs_resource, cross_id, implant_pt_vol);
  }

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("PanoModule::HoveredImplantInCSPlane");
#endif 
}

void PanoModule::SetIsImplantWire(const bool wire)
{
	ImplantSliceRenderer& implant_renderer = RendererManager::GetInstance().renderer_implant();
	implant_renderer.set_is_wire(wire);
}

const bool PanoModule::GetIsImplantWire()
{
	ImplantSliceRenderer& implant_renderer = RendererManager::GetInstance().renderer_implant();
	return implant_renderer.is_wire();
}
