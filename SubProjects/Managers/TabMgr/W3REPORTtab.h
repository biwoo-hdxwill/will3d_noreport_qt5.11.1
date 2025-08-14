#pragma once
/*=========================================================================

File:			class CW3REPORTtab
Language:		C++11
Library:		Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-08-29
Last modify:	2016-08-29

=========================================================================*/
#include <qboxlayout.h>

#include "../../Engine/Common/Common/W3Enum.h"

#include "base_tab.h"

class CW3ReportWindow;

class CW3REPORTtab : public BaseTab {
	Q_OBJECT

public:
	CW3REPORTtab();
	virtual ~CW3REPORTtab(void);

	virtual void SetVisibleWindows(bool isVisible) override;

public:
	void addThumbnail(const QString &path);

	void ApplyPreferences() {};

private:
	virtual void Initialize() override;
	virtual void SetLayout() override;

private:
	QVBoxLayout*		m_pLayout = nullptr;
	CW3ReportWindow*	m_pReport = nullptr;
	QRect report_rect_;
};
