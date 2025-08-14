#pragma once
/*=========================================================================

File:			class CW3LineItem_MPR
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-23
Last modify:	2015-12-19

=========================================================================*/
#include <qgraphicsitem.h>
#include <qvector2d.h>

#include "../../Common/Common/W3Enum.h"

#include "uiprimitive_global.h"

class QGraphicsLineItem;
class QGraphicsScene;
class CW3RectItem_rangeMPR;

enum EREAL_LINE { realline_unknown = -1, L, R, realline_end };

class UIPRIMITIVE_EXPORT CW3LineItem_MPR : public QObject {
  Q_OBJECT
 public:
  CW3LineItem_MPR(const UILineType eLineType, const MPRViewType eViewType);
  ~CW3LineItem_MPR();

  inline UILineType lineType(void) const { return m_eLineType; }

  // void	setVisibleTxt(bool bVisible);
  void setVisibleLines(bool bVisible);
  void setVisibleThickness(bool bVisible);
  void setThickness(float fThickness);

  void initThicknessBox(void);
  float thickness(void) const;

  inline QPointF pos(void) { return m_pLine[EREAL_LINE::L]->pos(); }
  void setPos(float fX, float fY);
  inline QPointF getPoint(void) const { return m_ptCenter; }
  inline void setPoint(const QPointF& ptCenter) { m_ptCenter = ptCenter; }
  inline void setNormal(const QVector2D& vNorm) { m_vNormal = vNorm; }
  inline const QVector2D& getNormal() { return m_vNormal; }
  inline const float& getAngleDegree() const { return angle_degree; }

  void setLine(const QPointF& ptCenter, const float mTranslate,
               const float fTranslate);
  void addLineToScene(QGraphicsScene* pScene);
  void removeLineToScene(QGraphicsScene* pScene);
  void initLine(const QPointF& ptCenter);
  void centeringLine(const QPointF& ptCenter);
  void initLineWidthColor(void);
  void setLineWidth(const float fWidth);
  void setLineColor(const QColor& color);
  void setThicknessAlpha(const float fAlpha);
  void setThicknessLineWidth(const float fWidth);

  void setLineRange(float fTrans, float fRotate);
  bool isSelectedBy(const QPointF& ptCur, const QPointF& ptCenter);
  bool isTranslationSelectedBy(const QPointF& ptCur, const QPointF& ptCenter);
  bool isRotationSelectedBy(const QPointF& ptCur, const QPointF& ptCenter);

  void rotate(const float& degree);
  float getRotateAngle(void) const;

  void rotateVector2D(const float& fAngle);

  void transformItems(const QTransform& transform);

 private:
  QVector2D rotateVector2D(const QVector2D& v, const float& fAngle);

 private:
  QGraphicsLineItem* m_pLine[EREAL_LINE::realline_end];
  QGraphicsLineItem* m_pThicknessLine[EREAL_LINE::realline_end];
  // QGraphicsTextItem*	m_pInteractionTxt;
  CW3RectItem_rangeMPR* m_pRangeMPR;

  UILineType m_eLineType;
  MPRViewType m_eViewType;

  int m_nRadius;
  float angle_degree;
  float m_fMarginTranslate;

  QVector2D m_vNormal;
  QPointF m_ptCenter;

  float m_fRangeTranslate;
  float m_fRangeRotate;
};
