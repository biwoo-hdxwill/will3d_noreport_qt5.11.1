#pragma once
/**=================================================================================================

Project:		UIFrame
File:			window_lightbox.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-10-08
Last modify: 	2018-10-08

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <QDoubleSpinBox>

#include <Engine/Common/Common/W3Enum.h>
#include "window.h"
#include "uiframe_global.h"

class UIFRAME_EXPORT WindowLightbox : public Window {
	Q_OBJECT

public:
	explicit WindowLightbox(const MPRViewType& view_type,
							const float& interval, const float& thickness,
							QWidget *parent = 0);
	virtual ~WindowLightbox();

	WindowLightbox(const WindowLightbox&) = delete;
	WindowLightbox& operator=(const WindowLightbox&) = delete;

public:
	void InitInterval(const float& interval);
	const int GetThicknessValue() const;

signals:
	void sigChangeThickness(double);
	void sigChangeInterval(double);
	void sigLightboxOff();
	void sigChangeLightboxLayout(int row, int col);

private slots:
	virtual void slotSelectLayout(int row, int column) override;
  void slotIntervalChanged(double slider_value);

private:
	virtual void Initialize() override;
	void Connections();

private:
	std::unique_ptr<QDoubleSpinBox> thickness_;
	std::unique_ptr<QDoubleSpinBox> interval_;
};
