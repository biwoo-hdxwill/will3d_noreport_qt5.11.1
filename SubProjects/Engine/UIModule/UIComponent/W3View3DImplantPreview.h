#pragma once
/*=========================================================================

File:			class CW3View3DImplantPreview
Language:		C++11
Library:		Qt 5.2.0
Author:			SANG KEUN PARK
First date:		2016-05-31

comment:
Implant Dialog에서 임플란트 미리보기용
=========================================================================*/
#include "uicomponent_global.h"

#include "w3view3d.h"

class CW3Implant;

class UICOMPONENT_EXPORT CW3View3DImplantPreview : public CW3View3D {
	Q_OBJECT

public:
	CW3View3DImplantPreview(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
							CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
							QWidget *pParent = 0);
	~CW3View3DImplantPreview();

	void setImplant(const QString& manufacturerName, const QString& productName,
					const QString& implantPath, float diameter, float length);

	virtual void init();
	inline CW3Implant* implant() const { return m_pImplant; }

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void drawBackground(QPainter *painter, const QRectF &rect);// 갱신시 화면 출력
	virtual void render3D() {}	// bypass 를 위한 빈 함수

	virtual void setInitScale();

private:
	CW3Implant * m_pImplant = nullptr;
	glm::vec3 m_NormFactor;
};
