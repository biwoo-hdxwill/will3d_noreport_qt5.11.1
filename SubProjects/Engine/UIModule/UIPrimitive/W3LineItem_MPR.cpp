#include "W3LineItem_MPR.h"
/*=========================================================================

File:			class CW3LineItem_MPR
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-23
Last modify:	2015-12-19

=========================================================================*/
#include <math.h>

#include <qfont.h>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qpen.h>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/color_will3d.h"
#include "W3RectItem_rangeMPR.h"

CW3LineItem_MPR::CW3LineItem_MPR(const UILineType eLineType,
                                 const MPRViewType eViewType)
    : m_eLineType(eLineType),
      m_eViewType(eViewType),
      m_nRadius(15),
      m_fRangeTranslate(160),
      m_fRangeRotate(200) {
  m_pRangeMPR = new CW3RectItem_rangeMPR(eLineType, eViewType, 0.0f);
  m_pRangeMPR->setVisible(false);

  // m_pInteractionTxt = new QGraphicsTextItem;
  // QFont font;
  // font.setStyleHint(QFont::SansSerif);
  // font.setPointSizeF(11);
  // font.setBold(true);
  // font.setItalic(true);
  // font.setStyleStrategy(QFont::PreferAntialias);
  // m_pInteractionTxt->setFont(font);

  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    m_pLine[idx] = new QGraphicsLineItem;
    m_pLine[idx]->setZValue(0.0);
    m_pThicknessLine[idx] = new QGraphicsLineItem;
    m_pThicknessLine[idx]->acceptHoverEvents();
    m_pThicknessLine[idx]->setOpacity(1.0f);
  }

  // m_pInteractionTxt->setFlags(QGraphicsTextItem::ItemIgnoresTransformations);
  m_fMarginTranslate = 0.0f;
}

CW3LineItem_MPR::~CW3LineItem_MPR() {
  // SAFE_DELETE_OBJECT(m_pInteractionTxt);
  SAFE_DELETE_OBJECT(m_pRangeMPR);

  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    SAFE_DELETE_OBJECT(m_pLine[idx]);
    SAFE_DELETE_OBJECT(m_pThicknessLine[idx]);
  }
}

//////////////////////////////////////////////////////////////////////////
//	public functions
//////////////////////////////////////////////////////////////////////////
void CW3LineItem_MPR::initLine(const QPointF& ptCenter) {
  angle_degree = 0.0f;
  m_ptCenter = ptCenter;
  this->setPos(ptCenter.x(), ptCenter.y());

  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    m_pLine[idx]->setVisible(true);
    m_pThicknessLine[idx]->setVisible(false);
    m_pLine[idx]->setRotation(angle_degree);
    m_pThicknessLine[idx]->setRotation(angle_degree);
  }

  m_pRangeMPR->setThickness(0.0f);
  m_pRangeMPR->setRotation(angle_degree);
  m_pRangeMPR->setVisible(false);

  // QString str;
  // switch (m_eViewType) {
  // case EMPR_VIEW_TYPE::axial:		str = "Axial";		break;
  // case EMPR_VIEW_TYPE::sagittal:	str = "Sagittal";	break;
  // case EMPR_VIEW_TYPE::coronal:	str = "Coronal";	break;
  //}
  // m_pInteractionTxt->setPlainText(str);
  // m_pInteractionTxt->setVisible(false);

  switch (m_eLineType) {
    case UILineType::HORIZONTAL:
      m_vNormal = QVector2D(0, 1);
      // m_pInteractionTxt->setRotation(0);
      break;
    case UILineType::VERTICAL:
      // m_pInteractionTxt->setRotation(90);
      m_vNormal = QVector2D(1, 0);
      break;
  }
}

void CW3LineItem_MPR::centeringLine(const QPointF& ptCenter) {
  angle_degree = 0.0f;
  m_ptCenter = ptCenter;
  this->setPos(ptCenter.x(), ptCenter.y());

  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    m_pLine[idx]->setRotation(angle_degree);
    m_pThicknessLine[idx]->setRotation(angle_degree);
  }

  m_pRangeMPR->setThickness(0.0f);
  m_pRangeMPR->setRotation(angle_degree);
  m_pRangeMPR->setVisible(false);

  switch (m_eLineType) {
    case UILineType::HORIZONTAL:
      m_vNormal = QVector2D(0, 1);
      break;
    case UILineType::VERTICAL:
      m_vNormal = QVector2D(1, 0);
      break;
  }
}

void CW3LineItem_MPR::setThickness(float fThickness) {
  m_pRangeMPR->setRectThickness(fThickness, m_pRangeMPR->length());
}
void CW3LineItem_MPR::initThicknessBox(void) {
  m_pRangeMPR->setThickness(0.0f);
  m_pRangeMPR->setVisible(false);
}

float CW3LineItem_MPR::thickness() const { return m_pRangeMPR->thickness(); }
void CW3LineItem_MPR::setVisibleLines(bool bVisible) {
  m_pLine[EREAL_LINE::L]->setVisible(bVisible);
  m_pLine[EREAL_LINE::R]->setVisible(bVisible);
  bool range_visibility = m_pRangeMPR->thickness() > 0.0 ? bVisible : false;
  m_pRangeMPR->setVisible(range_visibility);
}
void CW3LineItem_MPR::setVisibleThickness(bool bVisible) {
  m_pThicknessLine[EREAL_LINE::L]->setVisible(bVisible);
  m_pThicknessLine[EREAL_LINE::R]->setVisible(bVisible);
}
void CW3LineItem_MPR::addLineToScene(QGraphicsScene* pScene) {
  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    pScene->addItem(m_pLine[idx]);
    pScene->addItem(m_pThicknessLine[idx]);
  }

  pScene->addItem(m_pRangeMPR);
}
void CW3LineItem_MPR::removeLineToScene(QGraphicsScene* pScene) {
  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    pScene->removeItem(m_pLine[idx]);
    pScene->removeItem(m_pThicknessLine[idx]);
  }

  pScene->removeItem(m_pRangeMPR);
}
void CW3LineItem_MPR::setPos(float fX, float fY) {
  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    m_pLine[idx]->setPos(fX, fY);
    m_pThicknessLine[idx]->setPos(fX, fY);
  }

  m_pRangeMPR->setPosition(fX, fY);
}

void CW3LineItem_MPR::setLine(const QPointF& ptCenter, const float mTranslate,
                              const float mRotate) {
  m_fMarginTranslate = mTranslate;
  float length = mTranslate * 20;
  float boxThickness = m_pRangeMPR->thickness();
  m_pRangeMPR->setRectThickness(boxThickness, length);

  // skpark 20151104 Demo del
  if (m_eLineType == UILineType::HORIZONTAL) {
    m_pLine[EREAL_LINE::L]->setLine(-length, ptCenter.y(), ptCenter.x(),
                                    ptCenter.y());
    m_pLine[EREAL_LINE::R]->setLine(ptCenter.x(), ptCenter.y(), length,
                                    ptCenter.y());
    m_pThicknessLine[EREAL_LINE::L]->setLine(
        ptCenter.x() - mRotate, ptCenter.y(), ptCenter.x() - mTranslate,
        ptCenter.y());
    m_pThicknessLine[EREAL_LINE::R]->setLine(
        ptCenter.x() + mTranslate, ptCenter.y(), ptCenter.x() + mRotate,
        ptCenter.y());
  } else {  // vertical line
    m_pLine[EREAL_LINE::L]->setLine(ptCenter.x(), -length, ptCenter.x(),
                                    ptCenter.y());
    m_pLine[EREAL_LINE::R]->setLine(ptCenter.x(), ptCenter.y(), ptCenter.x(),
                                    length);
    m_pThicknessLine[EREAL_LINE::L]->setLine(
        ptCenter.x(), ptCenter.y() - mRotate, ptCenter.x(),
        ptCenter.y() - mTranslate);
    m_pThicknessLine[EREAL_LINE::R]->setLine(
        ptCenter.x(), ptCenter.y() + mTranslate, ptCenter.x(),
        ptCenter.y() + mRotate);
  }

  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    QPen penLine, penThickness;
    penLine.setCosmetic(true);
    penThickness.setCosmetic(true);
    penThickness.setWidthF(5.0f);
    QColor colorLine(Qt::white);

    switch (m_eViewType) {
      case MPRViewType::AXIAL:
        colorLine = ColorView::kAxial;
        break;
      case MPRViewType::SAGITTAL:
        colorLine = ColorView::kSagittal;
        break;
      case MPRViewType::CORONAL:
        colorLine = ColorView::kCoronal;
        break;
    }

    QBrush brushLine(colorLine);
    penLine.setBrush(brushLine);
    penThickness.setColor(colorLine);

    m_pLine[idx]->setPen(penLine);
    m_pLine[idx]->setVisible(false);
    m_pThicknessLine[idx]->setPen(penThickness);
    m_pThicknessLine[idx]->setVisible(false);
  }
}

void CW3LineItem_MPR::initLineWidthColor(void) {
  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    QPen penLine = m_pLine[idx]->pen();
    QPen penThickness = m_pThicknessLine[idx]->pen();
    penLine.setWidthF(1.0f);  // skpark 20151104 Demo add
    m_pLine[idx]->setPen(penLine);

    penThickness.setWidthF(5);
    m_pThicknessLine[idx]->setPen(penThickness);
    m_pThicknessLine[idx]->setVisible(false);
  }
}
void CW3LineItem_MPR::setLineWidth(const float fWidth) {
  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    QPen pen = m_pLine[idx]->pen();
    pen.setWidthF(fWidth);
    m_pLine[idx]->setPen(pen);
  }
}
void CW3LineItem_MPR::setLineColor(const QColor& color) {
  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    QPen pen = m_pLine[idx]->pen();
    pen.setColor(color);
    m_pLine[idx]->setPen(pen);
  }
}
void CW3LineItem_MPR::setThicknessAlpha(const float fAlpha) {
  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++)
    m_pThicknessLine[idx]->setOpacity(fAlpha);
}
void CW3LineItem_MPR::setThicknessLineWidth(const float fWidth) {
  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    QPen pen = m_pThicknessLine[idx]->pen();
    pen.setWidthF(fWidth);
    m_pThicknessLine[idx]->setPen(pen);
  }
}
void CW3LineItem_MPR::setLineRange(float fTrans, float fRotate) {
  m_fRangeTranslate = fTrans;
  m_fRangeRotate = fRotate;
}
bool CW3LineItem_MPR::isSelectedBy(const QPointF& ptCur,
                                   const QPointF& ptCenter) {
  if (!m_pLine[EREAL_LINE::L]->isVisible())  // by jdk 160906
    return false;

  QVector2D pt(ptCur.x(), ptCur.y());
  QVector2D ptC(ptCenter.x(), ptCenter.y());
  QVector2D vDist(m_vNormal.y(), -m_vNormal.x());

  if (pt.distanceToLine(ptC, vDist) < m_nRadius) {
    return true;
  } else {
    return false;
  }
}
bool CW3LineItem_MPR::isTranslationSelectedBy(const QPointF& ptCur,
                                              const QPointF& ptCenter) {
  if (!m_pLine[EREAL_LINE::L]->isVisible())  // by jdk 160906
    return false;

  QVector2D pt(ptCur.x(), ptCur.y());
  QVector2D ptC(ptCenter.x(), ptCenter.y());

  return (ptC.distanceToPoint(pt) < m_fRangeTranslate) ? true : false;
}
bool CW3LineItem_MPR::isRotationSelectedBy(const QPointF& ptCur,
                                           const QPointF& ptCenter) {
  if (!m_pLine[EREAL_LINE::L]->isVisible())  // by jdk 160906
    return false;

  QVector2D pt(ptCur.x(), ptCur.y());
  QVector2D ptC(ptCenter.x(), ptCenter.y());

  return (ptC.distanceToPoint(pt) > m_fRangeRotate) ? true : false;
}

void CW3LineItem_MPR::rotate(const float& degree) {
  angle_degree += degree;
  if (angle_degree > 360) angle_degree -= 360;
  if (angle_degree < 0) angle_degree += 360;

  for (int idx = 0; idx < EREAL_LINE::realline_end; idx++) {
    m_pLine[idx]->setRotation(angle_degree);
    m_pThicknessLine[idx]->setRotation(angle_degree);
  }
  m_pRangeMPR->setRotation(angle_degree);
}

float CW3LineItem_MPR::getRotateAngle(void) const {
  float angle = angle_degree;
  if (m_eLineType == UILineType::VERTICAL) {
    angle += 90.0f;
    if (angle >= 360.0f) angle -= 360.0f;
  }
  return angle;
}

QVector2D CW3LineItem_MPR::rotateVector2D(const QVector2D& v,
                                          const float& fAngle) {
  float cos_angle = cos(fAngle);
  float sin_angle = sin(fAngle);
  return QVector2D(cos_angle * v.x() - sin_angle * v.y(),
                   sin_angle * v.x() + cos_angle * v.y());
}

void CW3LineItem_MPR::rotateVector2D(const float& fAngle) {
  float cos_angle = cos(fAngle);
  float sin_angle = sin(fAngle);
  QVector2D tmpVec(cos_angle * m_vNormal.x() - sin_angle * m_vNormal.y(),
                   sin_angle * m_vNormal.x() + cos_angle * m_vNormal.y());

  m_vNormal.setX(tmpVec.x());
  m_vNormal.setY(tmpVec.y());

  // printf("(%f, %f), ", m_vNormal.x(), m_vNormal.y());
}

void CW3LineItem_MPR::transformItems(const QTransform& transform) {
  for (int i = 0; i < EREAL_LINE::realline_end; i++) {
    m_pLine[i]->setPos(transform.map(m_pLine[i]->pos()));
    m_pThicknessLine[i]->setPos(transform.map(m_pThicknessLine[i]->pos()));
  }

  m_pRangeMPR->transformItems(transform);
  // m_pInteractionTxt->setPos(transform.map(m_pInteractionTxt->pos()));
}
