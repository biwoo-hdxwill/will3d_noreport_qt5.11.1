#pragma once

/*=========================================================================

File:			class MeasureLine
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			JUNG DAE GUN
First Date:		2020-08-18
Modify Date:	2020-08-18

=========================================================================*/
#include <memory>
#include "measure_base.h"

class CW3PathItem_anno;

class UIPRIMITIVE_EXPORT MeasureLine : public MeasureBase
{
public:
	MeasureLine();
	virtual ~MeasureLine();

public:
	void setVisible(bool bShow) override;
	void setSelected(bool bSelected) override;
	void setNodeDisplay(bool bDisplay) override;

	void processMouseMove(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	void processMouseReleased(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override {}
	void processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override {}
	bool InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) override;

	virtual bool IsSelected() const override;

	virtual bool TransformItems(const QTransform& transform) override;

	virtual std::vector<QPointF> GetMeasurePoints() const override;

	virtual void ApplyPreferences() override;

private:
	std::unique_ptr<CW3PathItem_anno> line_;
};
