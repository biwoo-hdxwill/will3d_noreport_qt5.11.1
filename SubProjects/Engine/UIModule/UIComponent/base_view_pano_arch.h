#pragma once

/**=================================================================================================

Project:		UIComponent
File:			base_view_pano_arch.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-11
Last modify: 	2018-04-11

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#if defined(__APPLE__)
#include <glm/detail/type_vec3.hpp>
#else
#include <GL/glm/detail/type_vec3.hpp>
#endif
#include "../../Common/Common/W3Enum.h"
#include "view.h"
#include "uicomponent_global.h"

class ViewControllerSlice;
class ImplantTextItem;
class PanoArchItem;

class UICOMPONENT_EXPORT BaseViewPanoArch : public View {
	Q_OBJECT

public:
	explicit BaseViewPanoArch(QWidget* parent = 0);
	virtual ~BaseViewPanoArch();

	BaseViewPanoArch(const BaseViewPanoArch&) = delete;
	BaseViewPanoArch& operator=(const BaseViewPanoArch&) = delete;

signals:
	void sigTranslateZ(float z_pos_vol);
	void sigUpdatedArch();
	void sigUpdatedFinishArch();
	void sigDeletedArch();
	void sigShiftedArch(float shifted_value_in_vol);
	//void sigAutoArch();
	void sigRequestInitialize();
	void sigChangedArchRange(float range_mm);
	void sigChangedArchType(const ArchTypeID& type);

public:
	void SetCurrentArchType(const ArchTypeID& type);
	void SetPanoArchRange(float range_mm, bool is_emit_signal = true);
	void SetPanoArchThickness(float thciness_mm, bool is_emit_signal = true);
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	virtual void HideAllUI(bool is_hide) override;
	void CrossSectionUpdated();
	void UpdatedPanoRuler();
	void UpdateSlice();
	void SetHighlightCrossSection(int cross_id, bool is_highlight);
	void SetShiftedArch(float z_pos_vol);
	void ForceRotateMatrix(const glm::mat4& mat);
	void SetSliceInVol(float z_pos_vol);
	void SetArchSliceNumber(const ArchTypeID& type);
	void InitSliceRange(float min, float max, float value);
	void SetSliceRange(float min, float max);
	void SetArchPointsInVol(const std::vector<glm::vec3>& points);
	void SetArchPointsInVol(const std::vector<glm::vec3>& points, const ArchTypeID& type);

	void GetArchPointsInVol(std::vector<glm::vec3>& arch_points_in_vol) const;
	void GetArchCtrlPointsInVol(std::vector<glm::vec3>& arch_ctrl_points_in_vol) const;
	glm::vec3 GetUpVector() const;
	glm::vec3 GetCenterPosition() const;
	float GetArchRangeInVol() const;
	float GetArchShiftedInVol() const;
	float GetArchThicknessInVol() const;
	bool IsSetPanoArch() const;
	int GetSliceInVol() const;
	void GetSliceRange(int* min, int* max);

	inline const ArchTypeID& curr_arch_type() const { return curr_arch_type_; }

	virtual void ApplyPreferences() override;

	void ClearArch(const ArchTypeID& type);

protected slots:
	virtual void slotEndEditArch();
	virtual void slotActiveSharpenItem(const int index) override;

protected:
	void AddPointArch(const QPointF & pt_scene);
	void AddPointArch(const QPointF& pt_scene, const ArchTypeID& type);
	void ClearArch();
	void EndEditArch();

	virtual void TransformItems(const QTransform& transform) override;
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;

	virtual void leaveEvent(QEvent * event) override;
	virtual void enterEvent(QEvent * event) override;

	inline ViewControllerSlice* controller() const { return controller_.get(); }
	PanoArchItem* arch() const { return arch_[curr_arch_type_].get(); }
	PanoArchItem* arch(const int& i) const { return arch_[i].get(); }

	inline std::map<int, std::unique_ptr<ImplantTextItem>>& implant_specs() { return implant_specs_; }
	QPointF GetImplantSpecPosition(const int& implant_id, const bool& selected) const;
	void UpdateImplantSpecPosition();
	virtual void UpdateImplantHandlePosition() {}

	void RenderSlice();
	void UpdateMeasurePlaneInfo();
private slots:
	virtual void slotActiveFilteredItem(const QString & text) override;
	void SetFilteredItems();
	virtual void slotChangedValueSlider(int) override;
	void slotChangedArchRange();
	virtual void slotGetProfileData(const QPointF& start_pt_scene,
									const QPointF& end_pt_scene,
									std::vector<short>& data) override;
	virtual void slotGetROIData(const QPointF& start_pt_scene,
								const QPointF& end_pt_scene,
								std::vector<short>& data) override;

	void slotUpdateImplantSpecPosition(float);

private:
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

	virtual void SetGraphicsItems() override;
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;

	virtual void ActiveControllerViewEvent() override;

	void RequestDICOMInfo(const QPointF& pt_scene);
	void DisplayDICOMInfo(const glm::vec4& vol_info);

private:
	std::unique_ptr<ViewControllerSlice> controller_;
	std::unique_ptr<PanoArchItem> arch_[ArchTypeID::ARCH_TYPE_END];
	std::map<int, std::unique_ptr<ImplantTextItem>> implant_specs_;
	ArchTypeID curr_arch_type_ = ArchTypeID::ARCH_MANDLBLE;
	QString filtered_texts_[ArchTypeID::ARCH_TYPE_END];

	int drawn_arch_slice_[ArchTypeID::ARCH_TYPE_END] = { 0 };
};
