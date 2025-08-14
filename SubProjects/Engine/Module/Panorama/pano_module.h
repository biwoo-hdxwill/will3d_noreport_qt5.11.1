#pragma once

/**=================================================================================================

Project: 			Panorama
File:				pano_module.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-17
Last modify:		2017-07-17

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif
#include <QPointF>

#include "panorama_global.h"

class CW3Image3D;
class PanoResource;
class CrossSectionResource;
class SagittalResource;
class NerveResource;
class ImplantResource;

namespace panorama {
const glm::vec3 kInvalidVolPt = glm::vec3(-1.0f);
}

class PANORAMA_EXPORT PanoModule {
public:
	PanoModule(const PanoModule&) = delete;
	PanoModule& operator=(const PanoModule&) = delete;

public:
	static void ReconPanoPlane(const CW3Image3D& vol, PanoResource* pano_resource);
	static void ReconNerveMaskPano(const CW3Image3D & vol, PanoResource* pano_resource,
								   NerveResource* nerve_resource);
	static void ReconNerveMaskPanoThickness(const CW3Image3D & vol, PanoResource* pano_resource,
								   NerveResource * nerve_resource);
	static void ReconNerveMaskPanoXray(const CW3Image3D & vol, PanoResource * pano_resource, NerveResource * nerve_resource);
	static void ReconPanoThickness(const CW3Image3D& vol, PanoResource* pano_resource);
	static void ReconPanoXrayEnahancement(const CW3Image3D & vol, PanoResource * pano_resource);
	static void ReconPano3D(const CW3Image3D& vol, PanoResource* pano_resource);

	static void ReconPanoImplantMask(const CW3Image3D& vol, PanoResource* pano_resource, ImplantResource* implant_resource, float view_scale = 1.0f);

	static void MapVolToPanoPlane(const PanoResource& pano_resource, const std::vector<glm::vec3>& src_pts_vol,
								  std::vector<QPointF>& dst_pts_pano_plane);
	static QPointF MapVolToPanoPlane(const PanoResource& pano_resource, const glm::vec3& pt_vol);
	static QPointF MapVolToPanoPlane_Sagittal(const PanoResource& pano_resource, const glm::vec3& pt_vol);

	static void MapPanoPlaneToVol(const PanoResource& pano_resource,
								  const std::vector<QPointF>& src_pts_pano_plane,
								  std::vector<glm::vec3>& dst_pts_vol,
								  bool is_print_log = true);
	static glm::vec3 MapPanoPlaneToVol(const PanoResource& pano_resource,
									   const QPointF& pt_pano_plane,
									   bool is_print_log = true);

	static QPointF MapVolToCrossSectionPlane(const CrossSectionResource& cross_resource, int cross_id,
									  const glm::vec3& pt_vol);

	static void MapCrossSectionPlaneToVol(const CrossSectionResource & cross_resource, int cross_id,
										  const std::vector<QPointF>& src_pts_cross_plane,
										  std::vector<glm::vec3>& dst_pts_vol,
										  bool is_print_log = true);
	static glm::vec3 MapCrossSectionPlaneToVol(const CrossSectionResource & cross_resource, int cross_id,
											   const QPointF & pt_cross_section,
											   bool is_print_log = true);

	static QPointF MapVolToSagittalPlane(const SagittalResource& sagittal_resource,
										 const glm::vec3& pt_vol);
	
	static void MapSagittalPlaneToVol(const SagittalResource& sagittal_resource,
									  const std::vector<QPointF>& src_pts_sagittal_plane,
									  std::vector<glm::vec3>& dst_pts_vol,
									  bool is_print_log = true);
	static glm::vec3 MapSagittalPlaneToVol(const SagittalResource& sagittal_resource,
										   const QPointF& pt_sagittal, bool is_print_log = true);
	
	static void MapPano3DToVol(const PanoResource & pano_resource, const std::vector<glm::vec3>& src_pts_pano3d,
							   std::vector<glm::vec3>& dst_pts_vol);
	static glm::vec3 MapPano3DToVol(const PanoResource& pano_resource, const glm::vec3 & pt_pano3d);
	
	static void MapVolToPano3D(const PanoResource& pano_resource, const std::vector<glm::vec3>& src_pts_vol,
							   std::vector<glm::vec3>& dst_pts_pano3d);
	static glm::vec3 MapVolToPano3D(const PanoResource& pano_resource, const glm::vec3& pt_vol);

	static void GetPanoDirection(const PanoResource & pano_resource, const QPointF& pt_pano_plane,
								 glm::vec3& pt_prev, glm::vec3& pt_next);

	static void CheckCollideNerve(ImplantResource * implant_resource, const glm::mat4 & proj_view_mat);
	static void CheckCollideImplant(ImplantResource* implant_resource, const glm::mat4& proj_view_mat);
	static void HoveredImplantInPanoPlane(const PanoResource& pano_resource,
										  const ImplantResource& implant_resource,
										  const QPointF& pt_in_pano_plane, int* implant_id);
	static void HoveredImplantInSagittalPlane(const CW3Image3D& vol, const SagittalResource& sagittal_resource,
											  const ImplantResource& implant_resource,
											  const QPointF& pt_in_sagittal_plane, int* implant_id);
	static void HoveredImplantInCSPlane(const CW3Image3D & vol, const CrossSectionResource & cs_resource,
										const ImplantResource& implant_resource,
										int cross_id, const QPointF & pt_in_cs_plane,
										int & implant_id, QPointF & implant_pos);

	static void SetIsImplantWire(const bool wire);
	static const bool GetIsImplantWire();
};
