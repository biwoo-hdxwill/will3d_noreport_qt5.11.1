#pragma once
/**=================================================================================================
Project:		UIComponent
File:			pacs_view_pano.h
Language:		C++11
Library:		Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-07-29
Last modify: 	2021-08-11
Copyright (c) 2018 HDXWILL. All rights reserved.
 *===============================================================================================**/

#include "../../Common/Common/W3Enum.h"
#include "view.h"

class BaseViewController;
class ViewControllerPacsImage;
class UICOMPONENT_EXPORT PACSViewPano : public View
{
	Q_OBJECT
public:
	explicit PACSViewPano(QWidget* parent = 0);
	virtual ~PACSViewPano();

	PACSViewPano(const PACSViewPano&) = delete;
	PACSViewPano& operator=(const PACSViewPano&) = delete;

	void InitPano(bool nerve, bool implant);
	void UpdatedPano();
	void EmitCreateDCMFile(const int range);

	void SetCurPosValue(const int value);
	void SetNerveVisibility(bool is_visible);
	void SetImplantVisibility(bool is_visible);
	void SetSharpenLevel(const int level);

	virtual void ApplyPreferences() override;

	inline int get_cur_pos_value() { return cur_pos_value_; }

signals:
	void sigPanoUpdate(int value);
	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

private:
	void SetPanoImage();
	void RenderBackScreen();
	QString CreateDicomFileMiddlePath();

	virtual void ClearGL() override;
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

	virtual void InitializeController() override {};
	virtual bool IsReadyController() { return true; }
	virtual void ActiveControllerViewEvent() {}

private:
	ViewControllerPacsImage* controller_ = nullptr;

	int cur_pos_value_ = -1;

	bool init_pano_ = false;
	bool nerve_visible_ = false;
	bool implant_visible_ = false;
};
