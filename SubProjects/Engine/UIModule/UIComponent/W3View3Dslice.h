#pragma once
/*=========================================================================

File:			class CW3View3Dslice
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-03
Last modify:	2016-04-21

=========================================================================*/
#include "w3view3d.h"
#include "uicomponent_global.h"

class UICOMPONENT_EXPORT CW3View3Dslice : public CW3View3D {
	Q_OBJECT
public:
	CW3View3Dslice(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
				   CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
				   QWidget *pParent = 0);
	~CW3View3Dslice(void);

	virtual void reset();

protected:
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);

	virtual bool mouseMoveEventW3(QMouseEvent *event);	// by jdk 160720

	virtual void init();

	void setMVPslice();

	void renderGL();

protected:
	void equidistanceSpline(std::vector<glm::vec3> &out, std::vector<glm::vec3> &normal,
							std::vector<glm::vec3> &in, glm::vec3 &TopTranslate,
							glm::vec3 &upVector);
	void setInitScale();

	virtual void InvertView(bool bToggled) override;

protected:
	glm::mat4 m_invModel0;
	glm::mat4 m_invModel;
	glm::mat4 m_modelSlice;
	glm::mat4 m_volvertTotexCoord;
	glm::mat4 m_Ztranslate;
	glm::vec3 m_VolScale;

	float			m_curSlice = 0.0f;
	float			m_curSliceInVolume = 0.0f;

	////////////////////////////////////
	///////////// Flag For State
	bool m_bDoubleClickPressed = false;
	bool m_bIsXray = false;
};

