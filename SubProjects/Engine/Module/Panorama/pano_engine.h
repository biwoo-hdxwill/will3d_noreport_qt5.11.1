#pragma once

/**=================================================================================================

Project:		Panorama
File:			pano_engine.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-03-29
Last modify: 	2018-03-29

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <vector>
#include <map>
#include <memory>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include <QPointF>
#include <QColor>

#include "../../Common/Common/W3Enum.h"
#include "panorama_global.h"

class PanoResource;
class CrossSectionResource;
class SagittalResource;
class NerveResource;
class ImplantResource;
class CW3Image3D;
class ImplantData;
//20250123 LIN 주석처리
//#ifndef WILL3D_VIEWER
class ProjectIOPanoEngine;
//#endif

namespace implant_resource {
typedef struct _ImplantInfo ImplantInfo;
}

class PANORAMA_EXPORT PanoEngine {

public:
	PanoEngine();
	~PanoEngine();

	PanoEngine(const PanoEngine&) = delete;
	PanoEngine& operator=(const PanoEngine&) = delete;

	struct PanoROI {
		float top = 0.0f;
		float bottom = 0.0f;
		float slice = 0.0f;
	};

public:
	void Initialize(const CW3Image3D& volume);
	void InitSliceLocation(const CW3Image3D& volume);
	void InitCrossSectionResource();
	void InitSagittalResource();
	void InitPanoramaResource(const std::vector<glm::vec3>& pano_points,
                            const std::vector<glm::vec3>& pano_ctrl_points,
                            const glm::vec3& pano_back_vector, float pano_depth, float pano_range_mm, float pano_shifted,
                            float pano_thickness);

	void SetCurrentArchType(const ArchTypeID& type);

	void ShiftCrossSectionByImplant(int implant_id, const int& cross_section_id = 0);
	void EditCrossSectionShiftedValue(float delta);
	void EditCrossSectionShiftedValue(const glm::vec3& pt_vol);
	void EditNerveCtrlPoint(int nerve_id, int nerve_selected_index,
							const glm::vec3& point_in_vol);
	void EditNerveCtrlModifiedPoint(const glm::vec3& point_in_vol);

	void SetNerveParams(bool nerve_visible, int nerve_id, float nerve_radius, const QColor& nerve_color
						, bool is_update_resource = true);
	void UpdateNerveResource();
	void SetModifyNervePoint(const int& nerve_id, const int& nerve_selected_index, const bool& is_modify);
	void SetCrossSectionParams(float interval, int width, int height, float thickness,
							   float degree, int count);
	void SetCrossSectionNervePoint();
	void SetSagittalParams(int width, int height, float degree);
	void UpdateSagittal(const glm::vec3& point_in_vol);
	void SetPanoramaShiftedValue(int shifted_value);
	void SetPACSPanoShiftedValue(int shifted_value);

	void AddNerveCtrlPoint(int nerve_id, const glm::vec3& point_in_vol);
	void InsertNerveCtrlPoint(int nerve_id, int nerve_insert_index, const glm::vec3& nerve_point_in_vol);
	void RemoveNerveCtrlPoint(int nerve_id, int nerve_removed_index);
	void CancleNerveCtrlPoint(int nerve_id, int nerve_removed_index);
	void SetNerveVisibleAll(bool is_visible);
	void DeleteNerve(int nerve_id);

	void SetImplantMemo(const QString & text);
	bool AddImplant(const implant_resource::ImplantInfo& implant_params, int* prev_add_id);
	bool ChangeImplant(const implant_resource::ImplantInfo& implant_params);
	void SetImplantSelected(int implant_id, bool selected);
	void SetImplantVisibleAll(bool is_visible);
	void DeleteImplant(const int& implant_id);
	void DeleteAllImplants();

	void SetPanoROISlice(float value);
	void SetPanoROITop(float value);
	void SetPanoROIBottom(float value);

	void SetImplantAxisPointVolAndPano3D(int implant_id);
	void SetImplantAxisPointVol(int implant_id);
	void SetImplantAxisPointPano3D(int implant_id);
	void SetImplantAxisPointPanoPlane(int implant_id);

	void AppendImplantPosition(int implant_id, const glm::vec3& translate_vol);
	void SetImplantPositionVolAndPano3D(int implant_id, const glm::vec3& pt_vol);
	void SetImplantPositionInPanoPlane(int implant_id, bool is_update_z_position = true);

	void SetImplantPlaced();
	void UpdateImplantPositions();
	void TranslateImplantIn3D(int implant_id, const glm::vec3& delta_translate);
	void TranslateImplantInPano3D(int implant_id, const glm::vec3& delta_translate);
	void RotateImplantInPanoPlane(int implant_id, float delta_degree);
	void RotateImplantInSagittal(int implant_id, float delta_degree);
	void RotateImplantInCross(int implant_id, int cross_id, float delta_degree);
	void RotateImplantIn3D(int implant_id, const glm::vec3& rotate_axes, const float& delta_degree);
	void RotateImplantInPano3D(int implant_id, const glm::vec3 & rotate_axes, const float & delta_degree);

	int GetZValuePanorama(const glm::vec3& pt_vol);
	void MoveCrossSection(const glm::vec3& pt_vol);
	void ReconPanoramaPlane();
	void ReconPanoramaXrayEnhancement();
	void ReconPanorama3D();
	void ReconPanoramaNerveMask();
	void ReconPanoramaXrayNerveMask();
	void ReconPanoramaImplantMask(const float& view_scale);

	void ClearCrossSectionResource();
	void ClearPano3D();

	void GetNerveCtrlPointsInPanoPlane(std::map<int, std::vector<QPointF>>*ctrl_nerve_in_pano_plane) const;

	glm::vec3 MapPano3DToVol(const glm::vec3 & pt_pano3d) const;
	glm::vec3 MapVolToPano3D(const glm::vec3 & pt_vol) const;
	glm::vec3 MapPanoPlaneToVol(const QPointF& pt_in_pano_plane, bool is_print_log = true) const;
	QPointF MapVolToPanoPlane(const glm::vec3& vol_pos) const;
	QPointF MapVolToPanoPlane_Sagittal(const glm::vec3 vol_pos) const;
	glm::vec3 MapCrossPlaneToVol(int cross_id, const QPointF & pt_in_cross_plane,
								 bool is_print_log = true) const;
	glm::vec3 MapSagittalPlaneToVol(const QPointF& pt_in_sagittal_plane,
									bool is_print_log = true) const;
	QPointF MapVolToSagittalPlane(const glm::vec3 vol_pos) const;

	void SetReorienMat(const glm::mat4& mat);
	void SetReorienMat(const glm::mat4& mat, const ArchTypeID& arch_type);
	inline void set_projection_view_implant_3d(const glm::mat4& mat) { projection_view_implant_3d_ = mat; }


	QPointF MapVolToCrossPlane(int cross_id, const glm::vec3& pt_vol) const;
	
	void GetProfileDataInPanoPlane(const QPointF& start_pt_plane,
								   const QPointF& end_pt_plane,
								   std::vector<short>& data);
	void GetROIDataInPanoPlane(const QPointF& start_pt_plane,
							   const QPointF& end_pt_plane,
							   std::vector<short>& data);

	void GetPanoDirection(const QPointF& pt_pano_plane,
						  glm::vec3& pt_prev, glm::vec3& pt_next);

	void CheckCollideNerve();
	void CheckCollideImplant(const glm::mat4& projection_view_mat);
	void HoveredImplantInPanoPlane(const QPointF& pt_in_pano_plane, int* implant_id) const;
	void HoveredImplantInSagittalPlane(const QPointF& pt_sagittal_plane, int* implant_id) const;
	void HoveredImplantInCSPlane(const int& cross_id, const QPointF& pt_cs_plane, int& implant_id, QPointF& implant_pos) const;

	inline const PanoROI& GetPanoROI() const { return roi_vol_; }

	bool IsValidPanorama() const;
	bool IsValidPanoramaImage() const;
	bool IsValidNerve() const;
	bool IsValidImplant() const;
	bool IsValidCrossSection() const;
	bool IsValidSagittal() const;

	inline const glm::mat4& reorien_mat() const { return reorien_mat_[curr_arch_type_]; }
	inline const ArchTypeID& curr_arch_type() const { return curr_arch_type_; }

	const bool IsValidVolumePos(const glm::vec3 vol_pos) const;	

	// serialization

//20250214 LIN
//#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOPanoEngine& out, QString path = "");
//#endif
//20250123 LIN import 기능 viewer에 적용
	void importProject(ProjectIOPanoEngine& in);


	const float GetImplantRotateDegreeInPanoPlane(const int implant_id);

	void SetCrossSectionAngle(const int implant_id);

	void ResetDeltaShiftedValue();

	static void SetIsImplantWire(const bool wire);
	static const bool GetIsImplantWire();

	bool SetPanoThickness(const int thickness);

private:
	void SetImplantRotateInPanoPlaneFromVol(const ImplantData& implant_data);
	void SetImplantRotateInPanoFromVol(const ImplantData& implant_data);
	void SetImplantRotateInVolFromPano(const ImplantData& implant_data);
	void UpdateNerveResource(int nerve_id);

	void UpdateImplantPositionInPanoPlane();

	void SetCrossCenterPositionsInPanoPlane();
	void SetCrossSectionNerveCtrlPoint();
	void SetCrossSectionNerveProjection();
	void SetNerveMaskROI();
	void SetNervePoints(int nerve_id);
	void SetNerveMesh(int nerve_id);
	 
	const CW3Image3D& GetVolume() const;
	const glm::vec3 GetVolumePos(float z_pos_vol) const;

	inline PanoResource* res_pano() const { return res_pano_[curr_arch_type_].get(); }

private:
	std::shared_ptr<PanoResource> res_pano_[ArchTypeID::ARCH_TYPE_END];
	std::shared_ptr<CrossSectionResource> res_cross_section_;
	std::shared_ptr<SagittalResource> res_sagittal_;
	std::shared_ptr<NerveResource> res_nerve_;
	std::shared_ptr<ImplantResource> res_implant_;

	PanoROI roi_vol_;
	glm::mat4 reorien_mat_[ArchTypeID::ARCH_TYPE_END];
	glm::mat4 projection_view_implant_3d_;
	float cross_shifted_value_ = 0.0f;
	bool initialized_ = false;
	ArchTypeID curr_arch_type_ = ArchTypeID::ARCH_MANDLBLE;
};
