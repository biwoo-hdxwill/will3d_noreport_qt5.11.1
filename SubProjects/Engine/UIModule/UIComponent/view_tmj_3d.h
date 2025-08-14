#pragma once

/**=================================================================================================

Project:		UIComponent
File:			view_tmj_3d.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-12-07
Last modify: 	2018-12-07

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <Engine/Common/Common/W3Enum.h>

#include "view_3d.h"
#include "uicomponent_global.h"

class ViewControllerTMJ3D;

class UICOMPONENT_EXPORT ViewTMJ3D : public View3D 
{
	Q_OBJECT

public:
	ViewTMJ3D(const TMJDirectionType& type, QWidget* parent = 0);
	virtual ~ViewTMJ3D();

	ViewTMJ3D(const ViewTMJ3D&) = delete;
	ViewTMJ3D& operator=(const ViewTMJ3D&) = delete;

public:
	void UpdateCutting(const int& curr_step);
	void UpdateVR(bool is_high_quality);
	void ResetVolume();

	virtual void UpdateVolume() override;
	void UpdateFrontal();
	void SetCutting(const bool& cut_enable);

	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	virtual BaseViewController3D* controller_3d() override;

protected slots:
	virtual void slotRotateMatrix(const glm::mat4& mat) override;

protected:
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void SetWorldAxisDirection();

private:
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
	virtual void resizeEvent(QResizeEvent *pEvent) override;

	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void leaveEvent(QEvent * event) override;
	virtual void enterEvent(QEvent * event) override;
	virtual void wheelEvent(QWheelEvent * event) override;

	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void TransformItems(const QTransform& transform) override;
	virtual void ClearGL() override;
	void RenderVolume();
	void DisplayDICOMInfo(const QPointF & pt_scene);
	virtual void ActiveControllerViewEvent() override;

private:
	TMJDirectionType direction_type_ = TMJDirectionType::TMJ_TYPE_UNKNOWN;
	std::unique_ptr<ViewControllerTMJ3D> controller_;
};
