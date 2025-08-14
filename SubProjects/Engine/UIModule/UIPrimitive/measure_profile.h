#pragma once
/*=========================================================================

File:			class CW3ProfileAnnotation
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			JUNG DAE GUN, Seo Seok Man
First Date:		2015-09-17
Modify Date:	2018-05-17
Version:		2.0

Copyright (c) 2015 ~ 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <memory>
#include <vector>
#include <qwidget.h>
#include "measure_base.h"
#include "uiprimitive_global.h"

class ProfileLineItem;
class CW3ProfileDialog;

class UIPRIMITIVE_EXPORT MeasureProfile : public QWidget, public MeasureBase {
	Q_OBJECT
public:
	MeasureProfile();
	virtual ~MeasureProfile();

signals:
	void sigGetPlotterData(const QPointF& start_pt, const QPointF& end_pt,
						   std::vector<short>& data);

public:
	void setVisible(bool bShow) override;
	void setSelected(bool bSelected) override;
	void setNodeDisplay(bool bDisplay) override;
	void UpdateMeasure() override;

	void processMouseMove(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	void processMouseReleased(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	void processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override {}
	bool InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) override;

	virtual bool IsSelected() const override;

	virtual bool TransformItems(const QTransform& transform) override;

	void hideDialog();

	virtual std::vector<QPointF> GetMeasurePoints() const override;

	virtual void ApplyPreferences() override;
	virtual QString GetValue() override;

public slots:
	void slotPlotterWasClosed();

private slots:
	void slotShowAndUpdatePlotter();
	void slotShowPlotter();

private:
	void SetPlotterData(const std::vector<short>& data,
						const QPointF& start_pt, const QPointF& end_pt);

private:
	std::unique_ptr<ProfileLineItem> ui_line_;
	std::unique_ptr<CW3ProfileDialog> profile_dialog_;
};
