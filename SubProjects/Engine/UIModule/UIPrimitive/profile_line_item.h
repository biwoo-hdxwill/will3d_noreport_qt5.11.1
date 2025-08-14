#pragma once
/*=========================================================================

File:			class ProfileLineItem
Language:		C++11
Library:		Qt 5.9.9
Author:			JUNG DAE GUN
First date:		2021-03-05
Last modify:	2021-03-05

=========================================================================*/

#include "uiprimitive_global.h"

#include "W3PathItem_anno.h"

class UIPRIMITIVE_EXPORT ProfileLineItem : public CW3PathItem_anno
{
	Q_OBJECT
public:
	ProfileLineItem(
		QGraphicsScene* scene,
		common::measure::PathType type = common::measure::PathType::LINE
	);
	~ProfileLineItem();

public:
	virtual void AddPoint(const QPointF& point) override;
	virtual bool EndEdit() override;
	virtual void setVisible(bool bFlag) override;

signals:

public slots:
	void slotChangeLengthStartPos(const float percent_to_start);
	void slotChangeLengthEndPos(const float percent_to_start);

protected slots:
	virtual void slotTranslatePath(const QPointF& translate) override;

private slots:

public:

protected:
	virtual bool IsLineSelected() const;
	virtual void DrawingPath() override;
	virtual void GraphicItemsConnection() override;
	virtual void ApplyLineColor() override;

protected:

private:
	void SetFullLineColor(const QColor& color);
	void SetFullLineWidth(const float width);

private:
	CW3PathItem* length_line_ = nullptr;

	float start_pos_percent_ = 0.0f;
	float end_pos_percent_ = 1.0f;
};
