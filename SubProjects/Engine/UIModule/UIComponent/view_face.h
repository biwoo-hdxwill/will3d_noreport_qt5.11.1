#pragma once
/**=================================================================================================

Project: 			UIComponent
File:				view_face.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-20
Last modify:		2017-07-20

Copyright (c) 2017 ~ 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#if defined(__APPLE__)
#include <glm/detail/type_mat.hpp>
#else
#include <GL/glm/detail/type_mat.hpp>
#endif

#include "view_3d.h"
#include "uicomponent_global.h"

class QMenu;
class QAction;
class ViewControllerFace3D;

class UICOMPONENT_EXPORT ViewFace : public View3D {
	Q_OBJECT

public:
	explicit ViewFace(QWidget* parent = 0);
	~ViewFace();

	ViewFace(const ViewFace&) = delete;
	ViewFace& operator=(const ViewFace&) = delete;

signals:
	void sigRenderQuality();
	void sigRotateMat(const glm::mat4&);
	void sigSave3DFaceToPLYFile();
	void sigSave3DFaceToOBJFile();

public:
	void UpdateVRview(bool is_high_quality);
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	void SetTransparencyFacePhoto(float alpha);

	void SetVisibleFacePhoto(bool isVisible);
	void ForceRotateMatrix(const glm::mat4& mat);

	virtual BaseViewController3D* controller_3d() override;

	void LoadFace3D();

protected:
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

protected slots:
	virtual void slotRotateMatrix(const glm::mat4& mat) override;

private:
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
	virtual void resizeEvent(QResizeEvent *pEvent) override;

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;

	virtual void wheelEvent(QWheelEvent * event) override;

	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void TransformItems(const QTransform& transform) override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;

	void RenderVolume();

private:
	bool show_export_3d_face_menu_ = false;

	std::unique_ptr<ViewControllerFace3D> controller_;

	std::unique_ptr<QMenu> menu_;
	std::unique_ptr<QAction> action_save_3d_face_to_ply_;
	std::unique_ptr<QAction> action_save_3d_face_to_obj_;
};
