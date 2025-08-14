#pragma once
/*=========================================================================

File:			class CW3View2D_forMPR
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-11-21
Last date:		2016-06-04

=========================================================================*/
#define GLM_SWIZZLE
#include <GL/glew.h>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>
#else
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <gl/glm/glm.hpp>
#endif

#include <qgraphicsview.h>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/define_view.h"
#include "../../Common/Common/define_measure.h"

#include "uicomponent_global.h"

class QOpenGLWidget;
class QGraphicsProxyWidget;
class ViewNavigatorItem;
class CW3VREngine;
class CW3MPREngine;
class CW3ViewPlane;
class CW3ResourceContainer;
class CW3Render3DParam;
class ImageFilterSelector;
class CW3Slider_2DView;
class SimpleTextItem;
class CW3TextItem_ImplantID;
class CW3ViewRuler;
class GridLines;
class MeasureTools;
#ifndef WILL3D_VIEWER
class ProjectIOView;
#endif

namespace common {
namespace measure {
struct VisibilityParams;
}  // end of namespace measure
}  // end of namespace common

class UICOMPONENT_EXPORT CW3View2D_forMPR : public QGraphicsView {
  Q_OBJECT
 public:
  CW3View2D_forMPR(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
                   CW3ResourceContainer* Rcontainer, common::ViewTypeID eType,
                   QWidget* pParent = 0);

  virtual ~CW3View2D_forMPR();
  virtual void reset();

#ifndef WILL3D_VIEWER
  virtual void exportProject(ProjectIOView& out);
  virtual void importProject(ProjectIOView& in);
#endif

  virtual void drawImageOnViewPlane(bool isFitIn, int id, bool isCanalShown);

  void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
  virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

  void setWLWW();
  void changeWLWW();

  virtual void setVisible(bool state) override;
  inline float scaledSceneToVol(float f) {
    return f * m_scaleSceneToGL / m_scaleVolToGL;
  }

  QPointF GetViewCenterinScene(void) { return m_pntCurViewCenterinScene; }
  glm::vec4 getPlaneEquation();
  glm::vec3 getPlaneRightVector();
  glm::vec3 GetPlaneUpVector();
  inline common::ViewTypeID getViewType() { return m_eViewType; }

  bool IsDefaultInteractionsIn2D(Qt::MouseButton button);

  void ApplyPreferences();

  void VisibleObject(const VisibleID& visible_id, int state);
  virtual void VisibleNerve(int);
  virtual void VisibleImplant(int);
  virtual void VisibleSecond(int);
  virtual void VisibleFace(int);
  virtual void VisibleAirway(int) {}

  void VisibleVolMain(bool state);
  void VisibleVolSecond(bool state);
  void VisibleVolBoth(bool state);

  void InitFacePhoto3D();

  void SetMeasure3Dmode();
  void DeleteMeasureUI(const unsigned int& measure_id);
  void GetMeasureParams(const common::ViewTypeID& view_type,
                        const unsigned int& measure_id,
                        common::measure::VisibilityParams* measure_params);

  glm::mat4 GetProjectionViewMatrix() const;

#ifdef WILL3D_EUROPE
  inline void SetSyncControlButton(bool is_on) { is_control_key_on_ = is_on; }
#endif // WILL3D_EUROPE


 signals:
  void sigSyncScale(float);
  void sigSyncWindowing(int, int);

 protected:
  virtual void ResetView();
  virtual void FitView();
  virtual void InvertView(bool bToggled);
  void ScaleView(const QPoint& curr_view_pos);
  void HideText(bool bToggled);
  void HideUI(bool bToggled);
  virtual void HideMeasure(bool toggled);
  virtual void DeleteAllMeasure();
  virtual void DeleteUnfinishedMeasure();

  void PanningView(const QPointF& curr_scene_pos);
  void WindowingView(const QPointF& curr_scene_pos);

  virtual void leaveEvent(QEvent* event);
  virtual void enterEvent(QEvent* event);
  virtual void keyPressEvent(QKeyEvent* event);
  virtual void resizeEvent(QResizeEvent* pEvent) override;
  virtual void resizeScene();
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void mouseDoubleClickEvent(QMouseEvent* event);  // by jdk 160525
  virtual void wheelEvent(QWheelEvent* event);
  virtual void drawBackground(QPainter* painter, const QRectF& rect);
  void setImplantViewProjection();
  void updateMask();
  virtual void setProjection();
  virtual bool reconMPR(int id, bool isCanalShown);

  virtual void updateImage(){};
  virtual void changedDeltaSlider(int value){};
  virtual void clearGL();  // thyoo 170131: clear for vao, vbo

  virtual void setInitScale();

  void setVAOVBOImplant(int idx);
  void drawImplantID(int index, const QPointF& implantPos, bool bVisible);
  void drawImplantID(bool bVisible);
  void drawImageOnTexture(CW3ViewPlane* viewPlane, bool isDifferentSize);
  void CreateSlider();

  void setViewRulerValue();
  void SetGridValue();
  virtual QColor GetViewColor();

  // m_scaleSceneToGL : scaled GL coordinate
  inline float scaledVolToScene(const float& f) {
    return f * m_scaleVolToGL / m_scaleSceneToGL;
  }
  inline float scaledGLToVol(const float& f) { return f / m_scaleVolToGL; }
  inline float scaledVolToGL(const float& f) { return f * m_scaleVolToGL; }
  inline float scaledVolToScene() { return m_scaleVolToGL / m_scaleSceneToGL; }

  virtual void initVectorsUpBack();
  void transformItems(const QPointF& translate);
  void transformItems(const QPointF& preViewCenterInScene,
                      const QPointF& curViewCenterInScene, float scale);

  void transformPositionMemberItems(const QTransform& transform);
  virtual void transformPositionItems(const QTransform& transform){};
  void resetFromViewTool();
  void setViewProjection();

  void newQOpenGLWidget();
  void deleteQOpenGLWidget();

  void disconnections();
  virtual void SetVisibleItems();
  void SetVisibleItems(const bool visible);

  glm::vec3 MapSceneToVol(const QPointF& scene_pos);
  QPointF MapVolToScene(const glm::vec3& vol_pos);

 protected slots:
  void slotSelectFilter(const int index);
  void slotChangedValueSlider(int);
  void slotSetROIData(const QPointF& start_scene_pos,
                      const QPointF& end_scene_pos, std::vector<short>& data);
  void slotSetProfileData(const QPointF& start_scene_pos,
                          const QPointF& end_scene_pos,
                          std::vector<short>& data);

 private:
  void connections();
  void ToolTypeInteractions2DInMove(const QPoint& curr_view_pos);
  void GridOnOff(bool grid_on);

 protected:
  QOpenGLWidget* m_pGLWidget = nullptr;

  CW3Render3DParam* m_pRender3DParam = nullptr;

  CW3ResourceContainer* m_pgRcontainer = nullptr;
  CW3MPREngine* m_pgMPRengine = nullptr;
  CW3VREngine* m_pgVREngine = nullptr;

  // 0: main volume 에 대한 그리기, 1: second volume 에 대한 그리기
  CW3ViewPlane* m_pViewPlane[2] = {nullptr, nullptr};

  ViewNavigatorItem* m_pWorldAxisItem = nullptr;  //방위계

  // 아직 사용되는 곳 없음, 차후를 위해 놓아둠 -> SI에서 사용
  int m_drawVolId = 0;
  bool m_isDrawBoth = 0;  // second volume 과 같이 그릴 경우 true

  glm::vec3 m_vUpVec;  // monitor 뚫고 들어가는 방향
  glm::vec3 m_vBackVec;

  float m_fThickness[2];
  float m_scale = 1.0f;
  float m_scalePre = 1.0f;
  float m_scaleSceneToGL = 1.0f;
  float m_scaleVolToGL = 2.0f;
  float m_sceneWinView = 1.0f;
  float m_sceneHinView = 1.0f;
  float m_Wglorig = 1.0f, m_Hglorig = 1.0f, m_Dglorig = 1.0f;
  float m_Wglpre = 1.0f, m_Hglpre = 1.0f, m_Dglpre = 1.0f;
  float m_Wgl = 1.0f, m_Hgl = 1.0f, m_Dgl = 1.0f;
  float m_WglTrans = 0.0f, m_HglTrans = 0.0f;

  float m_initScale = 1.0f;

  common::ViewTypeID m_eViewType;
  common::ReconTypeID recon_type_ = common::ReconTypeID::MPR;

  // 매우 중요!! view 의 center 에 해당하는 scene 좌표
  QPointF m_pntCurViewCenterinScene = QPointF(0.0f, 0.0f);

  glm::mat4 m_model = glm::mat4(1.0f);
  glm::mat4 m_inverseScale;  // = inverse(m_scaleMat)
  glm::mat4 m_origModel;     // reorientation 으로 결정되는 행렬
  glm::mat4 m_rotate;        // 회전 행렬
  glm::mat4 m_scaleMat;  // W3View3D 의 m_vVolRange 를 glm::scale 한것과 같음
  glm::mat4 m_view;
  glm::mat4 m_projection = glm::mat4(1.0f);
  glm::mat4 m_mvp;

  glm::mat4 m_modelSecond =
      glm::mat4(1.0f);         // second volume 에 해당하는 model matrix
  glm::mat4 m_scaleMatSecond;  // second volume 에 대한 scale 행렬
  glm::mat4 m_mvpSecond;       // second volume 에 대한 mvp

  // implant 나 photo3D 를 wire 형식으로 그리기 위한 행렬
  glm::mat4 m_viewImplant;
  glm::mat4 m_projectionImplant;

  glm::mat4 m_modelPhoto;
  glm::mat4 m_modelPhotoForTexture;

  // view 및 scene 좌표
  QPoint last_view_pos_ = QPoint(0, 0);
  QPointF last_scene_pos_ = QPointF(0.0f, 0.0f);

  //	scene 과 gl 의 좌표계 오차를 위한 변수
  QPointF m_SceneGLOffset;

  ////////////////////////////////////
  ///////////// Flag For State
  bool m_isIntendedResize = false;  // 최초 resizeEvent 호출 이후 항상 true
  bool m_is2Dready = false;

  bool m_isUpdateSurface = false;
  bool m_isReconSwitched = false;
  bool m_isFacePhotoUpdated = false;

  common::CommonToolTypeOnOff common_tool_type_ =
      common::CommonToolTypeOnOff::NONE;

  MeasureTools* measure_tools_ = nullptr;

  CW3ViewRuler* ruler_ = nullptr;
  GridLines* grid_ = nullptr;
  CW3TextItem_ImplantID* implant_id_text_ = nullptr;
  SimpleTextItem* HU_value_ = nullptr;

  int m_nAdjustWindowWidth = 0;
  int m_nAdjustWindowLevel = 0;

  bool hide_all_view_ui_ = false;
  bool invert_windowing_ = false;

  bool m_bLoadProject = false;

  ImageFilterSelector* sharpen_filter_text_ = nullptr;
  unsigned int sharpen_level_ = 0;

  QGraphicsProxyWidget* m_pProxySlider = nullptr;
  CW3Slider_2DView* m_pSlider = nullptr;

  bool slider_value_set_ = true;
  int prev_slider_value_;

  QPointF m_sceneTrans = QPointF(0.0f, 0.0f);

  struct ImportProjectInfo {
    bool is_import = false;
    QPointF scene_center;
    float scene_to_gl = 0.0f;
    float gl_trans_x = 0.0f;
    float gl_trans_y = 0.0f;
    float view_scale = 1.0f;
  };

  ImportProjectInfo import_proj_info_;

  bool show_rulers_ = false;
  bool grid_visible_ = false;
  bool is_measure_delete_event_ = false;

  bool is_right_button_clicked_ = false;

  bool single_view_hide_ui_ = false;

#ifdef WILL3D_EUROPE
  bool is_control_key_on_ = false;
#endif // WILL3D_EUROPE
};
