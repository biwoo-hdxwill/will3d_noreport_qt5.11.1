#pragma once

/**=================================================================================================

Project: 			UIComponent
File:				view_pano.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-23
Last modify:		2017-08-23

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include "base_view_pano.h"
#include "uicomponent_global.h"

class PanoNerveItem;

class UICOMPONENT_EXPORT ViewPano : public BaseViewPano {
	Q_OBJECT
public:
	explicit ViewPano(QWidget* parent = 0);
	~ViewPano();

	ViewPano(const ViewPano&) = delete;
	ViewPano& operator=(const ViewPano&) = delete;

signals:
	void sigAddedNerveEllipse(int nerve_id, const QPointF& pt_pano_plane);
	void sigTranslatedNerveEllipse(int nerve_id, int nerve_selected_index, const QPointF& pt_pano_plane);
	void sigEndEditedNerve(int nerve_id);
	void sigClearedNerve(int nerve_id);
	void sigRemovedNerveEllipse(int nerve_id, int removed_index);
	void sigInsertedNerveEllipse(int nerve_id, int inserted_index, const QPointF& pt_pano_plane);
	void sigCancelLastNerveEllipse(int nerve_id, int removed_index);
	void sigModifyNerveEllipse(int nerve_id, int selected_index, bool is_modify);

public:
	void ReleaseSelectedNerve();
	void SetEditNerveMode(const bool& edit);
	void SetNerveHover(int nerve_id, bool is_hover);
	void SetVisibleNerve(int nerve_id, bool is_visible);
	void SetVisibleNerveAll(bool is_visible);
	void AddNerveEllipseFromCrossSection(const QPointF& point);
	void EndEditNerveFromCromssSection();
	void ClearUnfinishedNerve();
	void ClearNerve(int nerve_id);
	void ClearAllNerve();
	void PressedKeyESC();
	void UpdateNerveCtrlPoints(const std::map<int, std::vector<QPointF>>& ctrl_points_in_pano_plane);
	void GetNerveCtrlPointsInPanoPlane(int id, std::vector<QPointF>& dst_ctrl_points_in_pano_plane);
	int GetCurrentNerveID() const;

	virtual void UpdateImplantHandleAndSpec() override;
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	const bool& is_edit_nerve_mode() const { return is_edit_nerve_mode_; }

protected slots:
	virtual void slotActiveFilteredItem(const QString& text) override;

private:

	void InitializeNerveMenus();

	virtual void TransformItems(const QTransform& transform) override;

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	bool IsEventAddNerve() const;
	bool IsEventCancelLastNervePoint() const;
	bool IsEventEditNerve() const;
	bool IsEventNerveHovered() const;
	bool IsEventNerveHoveredLine() const;
	bool IsEventNerveHoveredPoint() const;

	void AddNervePoint(const QPointF& pt_scene);
	void EndEditNerve();
	void CancelLastNervePoint();


private slots:
	void slotDeleteNerveFromQAction();
	void slotRemovePointNerveFromQAction();
	void slotInsertPointNerveFromQAction();
	void slotAddedNerveEllipse(int nerve_id, const QPointF& pt_scene);
	void slotInsertedNerveEllipse(int nerve_id, int insert_index, const QPointF& pt_scene);
	void slotTranslatedNerveEllipse(int nerve_id, int nerve_selected_index, const QPointF& pt_scene);
	void slotMousePressedTimeOut();

private:
	std::unique_ptr<PanoNerveItem> nerve_;

	bool is_edit_nerve_mode_ = false;
	bool is_mouse_preseed_time_out_ = true;

	std::unique_ptr<QTimer> mouse_pressed_timer_;

	std::unique_ptr<QMenu> menu_nerve_ell_;
	std::unique_ptr<QMenu> menu_nerve_spl_;

	std::unique_ptr<QAction> menu_act_delete_nerve_;
	std::unique_ptr<QAction> menu_act_remove_pnt_nerve_;
	std::unique_ptr<QAction> menu_act_insert_pnt_nerve_;
};
