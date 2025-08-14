#include "W3RectItem_rangeMPR.h"
#include <qbrush.h>

#include "../../Common/Common/color_will3d.h"

CW3RectItem_rangeMPR::CW3RectItem_rangeMPR(UILineType eType, MPRViewType eViewType, float fThickness)
	: m_eLineType(eType) {
	QColor colorRect(Qt::white);

	switch (eViewType) {
	case MPRViewType::AXIAL:		colorRect = ColorView::kAxial;		break;
	case MPRViewType::SAGITTAL:	colorRect = ColorView::kSagittal;	break;
	case MPRViewType::CORONAL:	colorRect = ColorView::kCoronal;	break;
	}
	this->setBrush(QBrush(colorRect));
	this->setZValue(10);
	this->setOpacity(0.2f);
	this->setVisible(false);
	this->setRectThickness(fThickness, 800);
}

CW3RectItem_rangeMPR::~CW3RectItem_rangeMPR() {}

void CW3RectItem_rangeMPR::setRectThickness(float fThickness, float fBoxLength) {
	scale_ = 1.0f;
	m_fBoxLength = fBoxLength;
	m_fThickness = fThickness;

	this->SetRect();

	if (m_fThickness < 1.0f)
		this->setVisible(false);
	else
		this->setVisible(true);
}

void CW3RectItem_rangeMPR::setPosition(float pX, float pY) {
	setPos(pX, pY);
	this->setTransformOriginPoint(QPointF(0.0f, 0.0f));
}
void CW3RectItem_rangeMPR::SetRect() {
	float haf_thickness = (m_fThickness * scale_) * 0.5f;
	if (m_eLineType == UILineType::HORIZONTAL)
		setRect(-m_fBoxLength, -haf_thickness, m_fBoxLength * 2, haf_thickness*2.0f);
	else
		setRect(-haf_thickness, -m_fBoxLength, haf_thickness*2.0f, m_fBoxLength * 2);

}
void CW3RectItem_rangeMPR::transformItems(const QTransform & transform) {
	scale_ *= transform.m11();
	this->SetRect();
	setPos(transform.map(pos()));
}
