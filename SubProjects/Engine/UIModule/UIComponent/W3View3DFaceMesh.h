#pragma once
/*=========================================================================

File:			class CW3View3DFaceMesh
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-04-14
Last modify:	2016-04-14

=========================================================================*/
#include <memory>

#include "../UIGLObjects/W3SurfaceTextEllipseItem.h"
#include "W3VTOSTO.h"

#include "thyoo_W3View3D.h"
#include "uicomponent_global.h"

#define hdebug 0
class CW3SurfaceItem;
class CW3TRDsurface;
class ViewBorderItem;

class UICOMPONENT_EXPORT CW3View3DFaceMesh : public CW3View3D_thyoo {
  Q_OBJECT
 public:
  CW3View3DFaceMesh(CW3VREngine *VREngine, CW3MPREngine *MPREngine,
                    CW3VTOSTO *VTO, common::ViewTypeID eType,
                    QWidget *pParent = 0);
  ~CW3View3DFaceMesh(void);

 signals:
  void sigSetFaceMapping();

 public:
  void clearFace();

  void clearPoints();
  inline std::map<QString, glm::vec3> getMeshCtrlPoints() {
    return m_pEllipse->getPositions();
  }
  inline std::map<QString, int> getMeshTriangleIdxs() { return m_triangleIdxs; }

  virtual void reset();

  void generateFaceMesh(double);

 protected:
  virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
  void resizeEvent(QResizeEvent *event) override;

  virtual void mousePressEvent(QMouseEvent *event) override;
  virtual void mouseReleaseEvent(QMouseEvent *event) override;
  virtual void mouseMoveEvent(QMouseEvent *event) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual void keyReleaseEvent(QKeyEvent *event) override;

  virtual void setMVP(float rotAngle, glm::vec3 rotAxis) override;

  virtual void clearGL();

  void initializeGL(void);
  void renderingGL(void);

  virtual void resizeScene() override;

  void generateFaceMeshRunThread(double);

 protected:
  std::unique_ptr<ViewBorderItem> border_;
  CW3SurfaceItem *m_pFace = nullptr;
  CW3SurfaceTextEllipseItem *m_pEllipse = nullptr;
  std::map<QString, int> m_triangleIdxs;
  CW3VTOSTO *m_pgVTOSTO;
};
