#pragma once
/*=========================================================================

File:			class CW3View3DEndoSlice
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-13
Last date:		2016-06-17

=========================================================================*/
#include <memory>

#include "uicomponent_global.h"
#include "W3View3DEndo.h"

class ViewBorderItem;

///////////////////////////////////////////////////////////////////////////
//
//	* CW3View3DEndoSlice
//	좌상단 view
//	현재 카메라가 보는 방향으로 near 위치의 2D 단면을 rendering
//	CW3View3DEndo 에서 카메라를 이동하면 연동되어 같이 이동 하므로
//	상속받아 rendering만 2D로 변경
//	
///////////////////////////////////////////////////////////////////////////

class UICOMPONENT_EXPORT CW3View3DEndoSlice : public CW3View3DEndo {
	Q_OBJECT
public:
	CW3View3DEndoSlice(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
					   CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
					   QWidget *pParent = 0);
	~CW3View3DEndoSlice(void);

	virtual void reset();
	void reoriUpdate(glm::mat4* m);

public slots:
	//	* connected SIGNAL : CW3View3DEndo::sigSliceUpdate(const mat4)
	//	* const mat4 view : view matrix
	void slotSliceUpdate(const mat4 &view, const mat4 &invModel, int slicePos, int sliceRange);
	void slotSliceReset();

signals:
	//	* connected SLOT : CW3View3DEndo::slotWheelEvent(QWheelEvent *)
	//	* QWheelEvent * : wheel event
	void sigWheelEvent(QWheelEvent *);

protected:
	virtual void InvertView(bool bToggled) override;
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void wheelEvent(QWheelEvent *event);
	void SetNavigatorDirection();
	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void resizeScene();

	virtual void setProjection();
	virtual void init();

private:
	std::unique_ptr<ViewBorderItem> border_;
	glm::mat4 m_modelNealClipPlane;
	glm::mat4 m_inverseM;
};
