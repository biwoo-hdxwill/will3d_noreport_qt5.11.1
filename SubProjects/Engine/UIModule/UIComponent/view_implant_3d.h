#pragma once

/**=================================================================================================

Project:		UIComponent
File:			view_implant_3d.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-05-02
Last modify: 	2018-05-02

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "uicomponent_global.h"
#include "view_3d.h"

class ViewControllerImplant3D;
class ImplantTextItem;
class ImplantData;
class Measure3DManager;
#ifndef WILL3D_VIEWER
class ProjectIOView;
#endif

class UICOMPONENT_EXPORT ViewImplant3D : public View3D
{
	Q_OBJECT

public:
	explicit ViewImplant3D(QWidget* parent = 0);
	~ViewImplant3D();

	ViewImplant3D(const ViewImplant3D&) = delete;
	ViewImplant3D& operator=(const ViewImplant3D&) = delete;

#ifndef WILL3D_VIEWER
	void ExportProjectForMeasure3D(ProjectIOView& out);
	void ImportProjectForMeasure3D(ProjectIOView& in);
#endif

signals:
	void sigSelectImplant(int implant_id);
	void sigUpdateImplantImages(int implant_id);
	void sigTranslateImplant(const int& implant_id, const glm::vec3& delta_trans);
	void sigRotateImplant(const int& implant_id, const glm::vec3& rotate_axes,
		const float& delta_degree);
	void sigRotated();

public:
	void ResetVolume();
	void DeleteImplant(const int& implant_id);
	void DeleteAllImplants();
	void UpdateImplantSpec();
	void ChangeSelectedImplantSpec();
	void SyncBoneDensityRotateMatrix(const glm::mat4& rotate_mat);

	virtual void UpdateVolume() override;
	virtual void SetCommonToolOnOff(
		const common::CommonToolTypeOnOff& type) override;

	// Bone density view에서 현재 보고 있는 뷰와 일치시키기 위함
	const glm::mat4& GetViewMatrix() const;
	const glm::mat4& GetReorienMatrix() const;
	const glm::mat4& GetRotateMatrix() const;

	// 충돌체크를 위한 VP.
	glm::mat4 GetProjectionViewMatrix() const;

	virtual BaseViewController3D* controller_3d() override;

	virtual void ApplyPreferences() override;

	void Clip3DOnOff(const bool clip_on);

protected:
	virtual void DeleteUnfinishedMeasure() override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void SetWorldAxisDirection();

	virtual void HideAllUI(bool is_hide) override;

protected slots:
	virtual void slotRotateMatrix(const glm::mat4& mat) override;

private:
	void UpdateNotSelectedImplantSpec(
		const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
		int selected_id);
	void UpdateSelectedImplantSpec(
		const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
		int selected_id);

	void CreateImplantSpec(ImplantData* implant_data);

	bool IsEventImplantSelected() const;
	void SelectImplant();
	void UpdateImplantImages();
	void PickAndMoveImplant();
	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
	virtual void resizeEvent(QResizeEvent* pEvent) override;

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;

	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void TransformItems(const QTransform& transform) override;
	virtual void ClearGL() override;
	void RenderVolume();
	void RenderForPickAxes();
	virtual void ActiveControllerViewEvent() override;

	virtual void HideMeasure(bool toggled) override;
	virtual void DeleteAllMeasure() override;

	glm::vec3 VolumeToGLVertex(glm::vec3 volume_pos);

protected:
	Measure3DManager* measure_3d_manager_ = nullptr;

private:
	std::unique_ptr<ViewControllerImplant3D> controller_;
	std::map<int, std::unique_ptr<ImplantTextItem>> implant_specs_;
};
