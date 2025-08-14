#pragma once
/*=========================================================================

File:			class CW3View3DFacePhoto
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-04-14
Last modify:	2016-04-14

=========================================================================*/
#include <memory>

#include "thyoo_W3View2D.h"
#include "W3VTOSTO.h"

#include "uicomponent_global.h"

class CW3SurfaceTextEllipseItem;
class CW3SurfaceItem;
class ViewBorderItem;

class UICOMPONENT_EXPORT CW3View3DFacePhoto : public CW3View2D_thyoo {
	Q_OBJECT
public:
	CW3View3DFacePhoto(CW3VREngine *VREngine, CW3MPREngine *MPREngine,
					   CW3VTOSTO * VTO, common::ViewTypeID eType, QWidget *pParent = 0);
	~CW3View3DFacePhoto(void);

	void clearPhoto();

	virtual void reset();

signals:
	void sigSetFaceMapping();

public:
	void clearPoints();
	inline std::map<QString, glm::vec2> getTexCtrlPoints() { return m_texCtrlPoints; }
	inline QImage getPhotoImage() { return m_pPhotoImage; }

public slots:
	void slotLoadPhoto(const QString& FilePath);

protected:
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
	virtual void clearGL();
	virtual void resizeScene();
	void resizeEvent(QResizeEvent* event) override;

	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	void initializeGL(void);
	void initPhotoGL();
	void renderingGL(void);
	virtual void ResetView() override;
	virtual void FitView() override;

protected:
	std::unique_ptr<ViewBorderItem> border_;
	CW3SurfaceTextEllipseItem* m_pEllipse = nullptr;
	CW3SurfaceItem* m_pPhoto = nullptr;
	QImage m_pPhotoImage;
	std::map<QString, glm::vec2> m_texCtrlPoints;

	CW3VTOSTO* m_pgVTOSTO;

	unsigned int m_PROGpick;
	unsigned int m_PROGsurface;
	unsigned int m_PROGsurfaceTexture;

	bool m_is3Dready = false;
	bool m_isSetPhoto = false;
};
