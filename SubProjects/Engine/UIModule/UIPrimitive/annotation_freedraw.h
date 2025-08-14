#pragma once

/*=========================================================================

File:			class CW3FreeDrawAnnotation
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
#include <qobject.h>

class PathitemFreedraw;

class UIPRIMITIVE_EXPORT AnnotationFreedraw : public QObject, public MeasureBase
{
	Q_OBJECT

public:
	AnnotationFreedraw();
	virtual ~AnnotationFreedraw();

signals:
	void sigSelected(bool);

public:
	void setVisible(bool bShow) override;
	void setSelected(bool bSelected) override;
	void setNodeDisplay(bool bDisplay) override;

	void processMouseMove(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	void processMouseReleased(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	void processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override {}
	bool InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) override;

	virtual bool IsSelected() const override;

	virtual bool TransformItems(const QTransform& transform) override;
	virtual std::vector<QPointF> GetMeasurePoints() const override;

	QPolygonF GetPolygon();

	virtual void ApplyPreferences() override;

	void SetLineColor(const QColor& color);
	void SetLineWidth(const float width);

private:
	std::unique_ptr<PathitemFreedraw> ui_line_;
};
