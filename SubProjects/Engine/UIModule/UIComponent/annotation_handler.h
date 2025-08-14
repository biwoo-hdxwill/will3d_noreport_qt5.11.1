#pragma once
/*=========================================================================

File:			class AnnotationHandler
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			Seo Seok Man
First Date:		2018-02-02
Modify Date:	2018-05-15
Version:		2.0

Copyright (c) 2015~2018 All rights reserved by HDXWILL.

=========================================================================*/
#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#include <gl/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#endif

#include <qpoint.h>
#include <qobject.h>

#include <Engine\Common\Common\W3Enum.h>
#include <Engine\Common\Common\W3Define.h>
#include <Engine\UIModule\UIPrimitive\annotation_types.h>
#include "uiprimitive_global.h"

class QGraphicsScene;
class CW3Image2D;
class CW3BaseAnnotation;
class CW3ProfileAnnotation;

class UIPRIMITIVE_EXPORT AnnotationHandler : public QObject {
	Q_OBJECT

public:
	AnnotationHandler(QGraphicsScene *pScene) : scene_(pScene) {}
	virtual ~AnnotationHandler();

	// serialization
	void exportProject(QDataStream& out);
	void importProject(QDataStream& in, const int& version = VERSION_1_0_0);

	enum class ViewRenderMode { R2D, R3D };

private:
	enum class EventType {
		DRAW,
		END,
		SELECT,
		DEL,
		NONE
	};

	typedef std::vector<CW3BaseAnnotation *> AnnoList;

public:
	void ProcessMousePressed(const QPointF& pt, const glm::vec3& ptVol);
	void ProcessMouseMove(const QPointF& pt, const glm::vec3& ptVol);
	void ProcessMouseReleased(Qt::MouseButton button, const QPointF& pt, const glm::vec3& ptVol);
	void ProcessMouseDoubleClick(const QPointF& pt, const glm::vec3& ptVol);

	void DeleteAllAnno();
	void ClearUnfinishedItem();

	void SetVisible(bool visible);
	void SetViewRenderMode(const ViewRenderMode& mode);
	void SetCenter(const QPointF& point);
	void SetScale(const float scale);
	void SetPixelSpacing(float pixel_spacing);
	void SetAnnoType(ETOOL_TYPE toolType);

	void TransformItems(const QTransform& transform);

	void RemoveItems();
	void AddItems();

	void Update(const glm::vec3& vp_center, const glm::vec3& vp_up,
				const glm::vec3& vp_back);
	void UpdateProjection();

	const bool IsDrawing() const;
	const bool IsSelected() const;

	void SetProfileData(int anno_index, const std::vector<short>& data,
						const QPointF& start_pt, const QPointF& end_pt);

signals:
	void sigProtterUpdate();
	void sigGetProfileData(int anno_index, const QPointF& start_pt, const QPointF& end_pt);
	//void sigAnnotationDone(const anno::AnnotationType& type);

public slots:
	void slotPlotterWasClosed(CW3ProfileAnnotation *);
	void slotSyncSelectedStatus(bool);

private slots:
	void slotGetPlotterData(const QPointF& start_pt, const QPointF& end_pt);

private:
	void ActionDrawAnno(const QPointF& pt, const glm::vec3& ptVol);
	void ActionDeleteAnno(const QPointF& pt, const glm::vec3& ptVol);
	void ActionHighlightAnno();

	void BeginLengthMeasureInsert(const QPointF& pt, const glm::vec3& ptVol);
	void BeginTapeLineInsert(const QPointF& pt, const glm::vec3& ptVol);
	void BeginTapeCurveInsert(const QPointF& pt, const glm::vec3& ptVol);
	void BeginDegreeMeasureInsert(const QPointF& pt, const glm::vec3& ptVol);
	
	void BeginAreaLineInsert(const QPointF& pt, const glm::vec3& ptVol);
	void BeginProfileInsert(const QPointF& pt, const glm::vec3& ptVol);
	
	void BeginROIRect(const QPointF& pt, const glm::vec3& ptVol);
	void BeginROICircle(const QPointF& pt, const glm::vec3& ptVol);
	void BeginROIPolygon(const QPointF& pt, const glm::vec3& ptVol);

	void BeginRectangle(const QPointF& pt, const glm::vec3& ptVol);
	void BeginCircle(const QPointF& pt, const glm::vec3& ptVol);
	void BeginFreeDraw(const QPointF& pt, const glm::vec3& ptVol);
	void BeginArrowInsert(const QPointF& pt, const glm::vec3& ptVol);
	
	void BeginMemoInsert(const QPointF& pt, const glm::vec3& ptVol);
	void BeginCapture(const QPointF& pt);
	void AddAnno(CW3BaseAnnotation* obj, const QPointF& pt, const glm::vec3& ptVol);

	void NewAnno(const QPointF& pt, const glm::vec3& ptVol);
	void EndAnno();

	void DeleteLastAnno();
	void DeleteSelectedAnno();

	void CheckVisibility();
	void SelectEventType();

private:
	EventType event_type_ = EventType::NONE;
	anno::AnnotationType curr_anno_type_ = anno::AnnotationType::NONE;
	ViewRenderMode view_render_mode_ = ViewRenderMode::R2D;

	// hide measure 에 의해 visibility를 변경하기 위한 변수
	bool annotation_visible_ = true;
	AnnoList anno_list_;
	CW3BaseAnnotation* selected_anno_ = nullptr;
	QGraphicsScene * scene_ = nullptr;

	int profile_index_ = 0;
	int memo_index_ = 0;

	QPointF curr_view_center_;
	CW3Image2D *image_ = nullptr;
	float scale_ = 1.0f; // scene_to_gl / vol_to_gl
	float spacing_ = 1.0f;

	/*
		2D annotation 이 현재 보이는지를 체크하기 위한 변수들.
		Update 가 되기 전까지 이 값들을 이용하여 새로 입력되는 annotation 들의 
		위치 및 방향을 설정한다. 입력한 위치 및 방향 값은 변하지 않으며,
		annotation 의 고유 위치로 기억되어 변경된 view plane 의 위치와 비교하여
		annotation 들의 visibility 를 결정한다.
	*/
	glm::vec3 vp_center_;
	glm::vec3 vp_up_;
	glm::vec3 vp_back_;
};
