#pragma once
/**=================================================================================================

Project:		UIComponent
File:			base_view_orientation.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-14
Last modify: 	2018-02-14

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#if defined(__APPLE__)
#include <glm/detail/type_mat.hpp>
#else
#include <GL/glm/detail/type_mat.hpp>
#endif
#include <QTimer>

#include <Engine/Common/Common/W3Enum.h>
#include "view_3d.h"

class ViewControllerOrientation;
class PanoOrientationROIItem;

class UICOMPONENT_EXPORT BaseViewOrientation : public View3D {
	Q_OBJECT

public:
	explicit BaseViewOrientation(
		const common::ViewTypeID& view_type,
		const ReorientViewID& type,
		const bool enable_roi = true,
		QWidget* parent = 0
	);
	virtual ~BaseViewOrientation();

	BaseViewOrientation(const BaseViewOrientation&) = delete;
	BaseViewOrientation& operator=(const BaseViewOrientation&) = delete;

signals:
	void sigRenderQuality(const ReorientViewID&);
	void sigRotateMat(const ReorientViewID&, const glm::mat4&);
	void sigChangedROI();
	void sigChangedSlice();
	void sigUpdateDoneROI();

public:
	void ReadyROILines(float top, float slice, float bottom);
	void InitROILines();
	void SetSliceInVol(float z_pos_vol);
	void SetOrientationAngle(int degree, bool sign = true);
	void SetRotateMatrix(const glm::mat4& mat);
	void SetRenderQuality();
	void SetAngleDegree(int degree);
	int GetAngleDegree();

	float GetSliceInVol() const;
	float GetTopInVol() const;
	float GetBottomInVol() const;

	virtual BaseViewController3D* controller_3d() override;

protected slots:
	virtual void slotRotateMatrix(const glm::mat4& mat) override;

private:
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
	virtual void resizeEvent(QResizeEvent *pEvent) override;

	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent * event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;

	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void TransformItems(const QTransform& transform) override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;
	void RenderVolume();
	virtual void SetGraphicsItems() override;

	glm::mat4 GetRotateMatrix(int degree);

	bool IsHoveredROIitem();

private slots:
	void slotTranslateROILine(int id, float trans);
	void slotUpdate();

private:
	std::unique_ptr<ViewControllerOrientation> controller_;
	std::unique_ptr<QGraphicsPixmapItem> rotate_img_;
	ReorientViewID type_;

	std::unique_ptr<PanoOrientationROIItem> roi_item_;

	struct ROI {
		float top = 0.0f;
		float bottom = 0.0f;
		float slice = 0.0f;
	};
	ROI roi_vol_;

	bool is_ready_roi_ = false;
	bool is_init_roi_lines_ = false;
	bool is_changed_navigator_ = false;
	bool is_cleared_gl_ = false;

	QTimer timer_;
};
