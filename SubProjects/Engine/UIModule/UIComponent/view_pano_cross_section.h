#pragma once
/**=================================================================================================
Project:		UIComponent
File:			view_pano_cross_section.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-12-29
Last modify: 	2017-12-29
Copyright (c) 2017 HDXWILL. All rights reserved.
 *===============================================================================================**/

#include "uicomponent_global.h"
#include "base_view_pano_cross_section.h"

class CW3EllipseItem;
class UICOMPONENT_EXPORT ViewPanoCrossSection : public BaseViewPanoCrossSection 
{
	Q_OBJECT
public:
	ViewPanoCrossSection(int cross_section_id, QWidget* parent = 0);
	~ViewPanoCrossSection();

	ViewPanoCrossSection(const ViewPanoCrossSection&) = delete;
	ViewPanoCrossSection& operator=(const ViewPanoCrossSection&) = delete;

signals:
	void sigAddNerve(const int& cross_section_id, const QPointF& pt_cross_section_plane);
	void sigTranslatedNerve(const int& cross_section_id, const QPointF& pt_cross_section_plane);
	void sigEndEditNerve();
	void sigPressedKeyESC();

	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

public:
	void SetEditNerveMode(const bool& is_edit);
	virtual void UpdateCrossSection() override;
	void UpdateModifyNerveEll();

#ifndef WILL3D_VIEWER
	const int GetCrossSectionFilterLevel();
	void RequestedCreateDCMFiles(const QString& time, bool nerve_visible, bool implant_visible, int filter, int thickness);
#endif // !WILL3D_VIEWER

private:
	virtual void TransformItems(const QTransform& transform) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	bool IsEventAddNerve() const;
	bool IsEventEditNerve() const;

private slots:
	void slotTranslatedEllipse(const QPointF& pos);

private:
	std::unique_ptr<CW3EllipseItem> ellipse_nerve_;
	bool is_edit_nerve_mode_ = false;
	bool is_pressed_double_click_ = false;
};
