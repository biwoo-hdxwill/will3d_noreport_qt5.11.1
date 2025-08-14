#pragma once

/*=========================================================================

File:			class CW3TapeAnnotation
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:		2015-09-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <memory>
#include <qpolygon.h>

#include <Engine/Common/Common/define_measure.h>
#include "measure_base.h"

class CW3PathItem_anno;

class UIPRIMITIVE_EXPORT MeasureTape : public MeasureBase
{
public:
	MeasureTape(common::measure::PathType path_type, common::measure::DrawMode draw_mode);
	virtual ~MeasureTape();

public:
	void setVisible(bool bShow) override;
	void setSelected(bool bSelected) override;
	void setNodeDisplay(bool bDisplay) override;
	void UpdateMeasure() override;

	void processMouseMove(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	void processMouseReleased(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override {}
	void processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	bool InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) override;

	virtual bool IsSelected() const override;

	virtual bool TransformItems(const QTransform& transform) override;

	virtual std::vector<QPointF> GetMeasurePoints() const override;

	virtual void ApplyPreferences() override;
	virtual QString GetValue() override;

private:
	void processing(const QPointF& pt);
	void UpdateLength();

private:
	std::unique_ptr<CW3PathItem_anno> ui_line_;
	common::measure::PathType path_type_;
	common::measure::DrawMode draw_mode_;
	std::vector<float> length_list_;
};
