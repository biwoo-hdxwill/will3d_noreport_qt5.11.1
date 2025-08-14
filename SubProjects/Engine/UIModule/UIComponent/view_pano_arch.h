#pragma once
/**=================================================================================================

Project: 			UIComponent
File:				view_pano_arch.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-01
Last modify:		2017-08-01

 *===============================================================================================**/
#include "base_view_pano_arch.h"

class QMenu;
class QAction;
class CW3TextItem;

class UICOMPONENT_EXPORT ViewPanoArch : public BaseViewPanoArch
{
	Q_OBJECT
public:
	explicit ViewPanoArch(QWidget* parent = 0);
	~ViewPanoArch();

	ViewPanoArch(const ViewPanoArch&) = delete;
	ViewPanoArch& operator=(const ViewPanoArch&) = delete;

signals:
	void sigChangeEditArchMode();

public:
	void SetManualArchMode();
	bool SetApplyArch();
	inline bool is_edit_mode() const { return is_edit_mode_; }
	virtual void HideAllUI(bool is_hide) override;

private slots:
	void slotDeleteArchFromQAction();
	void slotRemovePointArchFromQAction();
	void slotInsertPointArchFromQAction();
	virtual void slotEndEditArch() override;
	bool slotPressedAdjustArchText();

private:
	void InitializeArchMenus();
	bool IsEventCancelLastArchPoint() const;

	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void mousePressEvent(QMouseEvent * event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	void DrawCurrentArch(const QPointF& pt_scene);

private:	
	std::unique_ptr<CW3TextItem> txt_adjust_arch_;

	std::unique_ptr<QMenu> menu_arch_ell_;
	std::unique_ptr<QMenu> menu_arch_spl_;

	std::unique_ptr<QAction> menu_act_delete_arch_;
	std::unique_ptr<QAction> menu_act_remove_pnt_arch_;
	std::unique_ptr<QAction> menu_act_insert_pnt_arch_;

	bool is_edit_mode_ = false;
};
