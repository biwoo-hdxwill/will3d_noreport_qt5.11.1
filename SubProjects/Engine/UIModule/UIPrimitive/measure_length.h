#pragma once
/*=========================================================================

File:			class CW3LengthAnnotation
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:		2015-06-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <memory>
#include "measure_base.h"

class CW3PathItem_anno;

class UIPRIMITIVE_EXPORT MeasureLength : public MeasureBase {
public:
	MeasureLength();
	virtual ~MeasureLength();

public:
	void setVisible(bool bShow) override;
	void setSelected(bool bSelected) override;
	void setNodeDisplay(bool bDisplay) override;
	void UpdateMeasure() override;

	void processMouseMove(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	void processMouseReleased(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override {}
	void processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override {}
	bool InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) override;

	virtual bool IsSelected() const override;

	virtual bool TransformItems(const QTransform& transform) override;

	virtual std::vector<QPointF> GetMeasurePoints() const override;

	virtual void ApplyPreferences() override;
	virtual QString GetValue() override;

private:
	const float GetPixelDistance(QPointF p1, QPointF p2);
	
private:
	std::unique_ptr<CW3PathItem_anno> line_;
	float pixel_dist_ = 0.0f;
};
