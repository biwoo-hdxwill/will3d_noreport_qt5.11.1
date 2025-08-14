#pragma once
/*=========================================================================

File:			class CW3ViewMPR
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-11-23
Last modify:	2016-04-21

=========================================================================*/
#include "../../Common/Common/w3_struct.h"
#include "W3View2D_forMPR.h"

#include "uicomponent_global.h"

class EllipseMPR;
class CW3LineItem_MPR;
class CW3TextItem;
class Zoom3DCube;
class CW3MPREngine;
class CW3VREngine;
class ImplantHandle;
#ifndef WILL3D_VIEWER
class ProjectIOMPR;
class ProjectIOSI;
#endif
class PanoArchItem;

namespace commmon
{
	enum ViewTypeID;
}  // namespace commmon

static int Line2View[MPRViewType::MPR_END][UILineType::LINE_END] =
{
	{MPRViewType::CORONAL, MPRViewType::SAGITTAL},  // Axial	: [Horizontal][Vertical]
	{MPRViewType::AXIAL, MPRViewType::CORONAL},		// Sagittal : [Horizontal][Vertical]
	{MPRViewType::AXIAL, MPRViewType::SAGITTAL}		// Coronal	: [Horizontal][Vertical]
};

namespace lightbox_resource
{
	typedef struct _PlaneParams PlaneParams;
}

class UICOMPONENT_EXPORT CW3ViewMPR : public CW3View2D_forMPR
{
	Q_OBJECT

public:
	CW3ViewMPR(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
		CW3ResourceContainer* Rcontainer, const common::ViewTypeID& eType,
		const MPRViewType& mpr_type, QWidget* pParent = 0);

	virtual ~CW3ViewMPR();

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOMPR& out);
	void importProject(ProjectIOMPR& in);
	void exportProject(ProjectIOSI& out);
	void importProject(ProjectIOSI& in);
#endif

	virtual void setVisible(bool state) override;

	void initViewPlane(int id);

	void SetThicknessMaximumRange(const float& range);
	void centeringItems(void);

	void InitialDraw();

	void rotateItems(const float& radian);
	void rotateDraw(const glm::vec3& UpVec, float fAngle);
	void rotate3DDraw(glm::mat4* T, glm::mat4* rotate);
	void rotate3DDrawPassive(const glm::mat4& rotate);
	float translateDraw(float fDist, const bool limit_move_range = true);
	void translateCrossPt(const UILineType eLineType, float fDist);
	void translateCrossPt(const float& fDistx, const float& fDisty);

	void thickDraw(const float& fThickness);
	void SetIntervalOfSlider(float interval);
	void thickLineChange(MPRViewType eLineType, float fThickness);

	void SetVisibleMPRUIs(bool visible);
	void SetVisibleRects(bool visible);
	void setDirectionText();

	void setZoom3DMPR(const float& radius, const bool updateVR);
	void rotateZoom3DCube(const glm::mat4& rotMat);

	inline void SIModeOn(const bool& enable) noexcept
	{
		super_imposition_mode_ = enable;
	}

	virtual void reset();

	void updateImageOnViewPlane();
	void updateMPRPhoto();
	void zoom3D(bool bToggled);

	virtual void VisibleImplant(int state) override;
	virtual void VisibleSecond(int state) override;
	void SecondUpdate(glm::mat4*);
	virtual void drawImageOnViewPlane(bool isFitIn, int id,
		bool isCanalShown) override;

	void ApplyPreferences();
	void WheelView(MPRViewType type, float dist);

	void GetViewPlaneParams(lightbox_resource::PlaneParams& plane_param);
	glm::vec3 GetViewPlaneCenter();
	glm::mat4 GetPlaneRotateMatrix() const;
	const float& GetAngleDegree() const;

	void AdjustImplant(bool checked);

	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	const QVector2D& GetLineNormal(UILineType line);

	void DrawArch(ArchTypeID arch_type);

	void initItems(void);

#ifdef WILL3D_EUROPE
	inline bool mouse_orver() { return mouse_orver_; }
#endif // WILL3D_EUROPE

signals:
	void sigRotate(UILineType, MPRViewType, const glm::vec3&, float);
	void sigTranslate(MPRViewType, MPRViewType, float);
	void sigTranslateCross(MPRViewType, QVector2D&);
	void sigThickChange(MPRViewType, MPRViewType, float);
	void sigMouseRelease();
	void sigWheel(MPRViewType, float, const bool update_3d_view);
	void sigSetZoom3DMPR(const MPRViewType eViewType, const QPointF center,
		const float radius, const bool updateVR);
	void sigSetZoom3DVR(const MPRViewType eViewType, const glm::vec3 center,
		const float radius);
	void sigSelectedMPR(MPRViewType, const bool update_3d_view);  // thyoo 161011

	void sigMeasureCreated(const unsigned int& measure_id);
	void sigMeasureDeleted(const unsigned int& measure_id);

	void sigKeyPressEvent(QKeyEvent*);

	void sigSelectImplant(const int id);
	void sigTranslateImplant(const int& implant_id, const glm::vec3& delta_trans);
	void sigRotateImplant(const int& implant_id, const glm::vec3& rotate_axes, const float& delta_degree);

	void sigDoneAdjustImplant();

	void sigUpdateArch(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number);
#ifdef WILL3D_EUROPE
	void sigSyncControlButton(bool);
	void sigShowButtonListDialog(const QPoint& window_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

public slots:
	void slotReoriupdate(glm::mat4*);
	void slotSyncScale(float scale);
	void slotSyncWindowing(int level, int width);

private:
#ifdef WILL3D_EUROPE
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;
#endif // WILL3D_EUROPE
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void resizeEvent(QResizeEvent* pEvent) override;
	virtual void resizeScene() override;
	void SetNavigatorDirection();
	void setSecondVectors();

	virtual void updateImage() override;
	void UpdateSliderValue();
	virtual void changedDeltaSlider(int delta) override;
	virtual void drawBackground(QPainter* painter, const QRectF& rect);
	virtual bool event(QEvent* event);

	virtual void transformPositionItems(const QTransform& transform) override;

	//void initItems(void);
	virtual void initVectorsUpBack() override;

	virtual QColor GetViewColor() override;
	float getRotationAngle(const QPointF& ptCur);
	void setMPRInteraction(bool bInteraction);
	void setPosItemsUseCrossCenter();

	virtual void ResetView() override;
	virtual void FitView() override;

	void RotateView(const QPointF& scene_pos);
	void ChangeTranslateCursorWithRotateAngle(const float angle);
	void ChangeUIInteractiveArea(UILineType line_type, const QPointF& scene_pos);
	void DrawImplantID();
	void DrawZoom3D(const QPointF& scene_pos);
	float GetCurrentCenterMargin();
	void GetMPREventType();

	void ChangeMPRInMove(const QPointF& scene_pos);
	void ChangeThicknessInMove(UILineType line_type, const QPointF& scene_pos);
	void ChangeCircleInMove(const QPointF& scene_pos);
	void ChangeZoom3DInMove(const QPointF& scene_pos);

	/**********************************************************************************************
	Move interaction zoom 3D.

	@author	Seo Seok Man
	@param	scene_pos	The scene position in move event.
	@return	True if move event was accepted.
	 **********************************************************************************************/
	bool MoveInteractionZoom3D(const QPointF& scene_pos);
	void MoveInteractionMPRItems(const QPointF& scene_pos);

	void DisplayDICOMInfo(const QPointF& scene_pos);

	void CreateDirectionText();
	virtual void SetVisibleItems() override;
	void SetVisibleItems(const bool visible);

	void SetSliceNumberPosition();

	void PickImplant(const QPointF& scene_pos);
	void SetImplantIDTextPosUnderImplantHandle();

private slots:
	void slotTranslateImplant();
	void slotRotateImplant(float degree_angle);
	void slotArchEndEdit();

private:
	enum MPRItemType { none, circle, hLine, vLine };

	enum class MPREventType
	{
		NONE,
		ROTATE,
		ZOOM3D,
		CROSS,
		TRANSLATE,
		THICKNESS,
		VIEW2D_EVENT
	};

private:
	MPRViewType m_eMPRType;
	MPRItemType selected_item_ = MPRItemType::none;
	MPREventType event_type_ = MPREventType::NONE;

	glm::vec3 m_vUpVecSecond;  // monitor 뚫고 들어가는 방향
	glm::vec3 m_vBackVecSecond;

	EllipseMPR* circle_ = nullptr;
	CW3LineItem_MPR* m_pLine[UILineType::LINE_END] = { nullptr, nullptr };

	QGraphicsRectItem* m_rect = nullptr;
	QGraphicsRectItem* m_rectShadow = nullptr;
	float m_marginCenterSqr = 0.0f;

	QPointF m_crossCenter = QPointF(0.0f, 0.0f);

	bool super_imposition_mode_ = false;
	bool m_bReoriupdated = false;

	float m_fThicknessMaxInVol = 0.0f;
	float interval_slider_ = 1.0f;

	Zoom3DCube* m_pZoom3D = nullptr;
	bool m_bIsZoom3DMode = false;

	glm::mat4 m_secondTransform;
	glm::mat4 m_reoriMat;

	CW3TextItem* slice_number_ = nullptr;
	CW3TextItem* direction_text_ = nullptr;

	bool show_slice_numbers_ = false;

	std::unique_ptr<ImplantHandle> implant_handle_;

	bool adjust_implant_ = false;

	bool draw_arch_ = false;
	PanoArchItem* arch_ = nullptr;
	ArchTypeID arch_type_ = ArchTypeID::ARCH_MANDLBLE;

	glm::mat4 orientation_matrix_;

#ifdef WILL3D_EUROPE
	bool mouse_orver_ = false;
#endif // WILL3D_EUROPE
};
