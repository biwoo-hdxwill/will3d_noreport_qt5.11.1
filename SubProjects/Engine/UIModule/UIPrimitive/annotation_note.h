#pragma once

/*=========================================================================

File:			class CW3MemoAnnotation
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:		2015-06-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <memory>
#include <qwidget.h>
#include "measure_base.h"

class CW3PathItem_anno;
class CW3InputDialog;

class UIPRIMITIVE_EXPORT AnnotationNote : public QWidget, public MeasureBase {
	Q_OBJECT

public:
	explicit AnnotationNote();
	virtual ~AnnotationNote();

signals:
	void sigSetNote(const QString& txt);

public:
	void setVisible(bool bShow) override;
	void setSelected(bool bSelected) override;
	void setNodeDisplay(bool bDisplay) override {}

	void processMouseMove(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	void processMouseReleased(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override {}
	void processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) override;
	bool InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) override;
	void InputParamWithResource(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol);

	virtual bool IsSelected() const override;
	virtual std::vector<QPointF> GetMeasurePoints() const override;

	QString NoteText();
	virtual bool TransformItems(const QTransform& transform) override;

	virtual void ApplyPreferences() override;
	virtual QString GetValue() override;

private slots:
  void slotFinishedNoteDlg();
private:
	std::unique_ptr<CW3PathItem_anno> ui_line_;
	std::unique_ptr<CW3InputDialog> input_dlg_;
};
