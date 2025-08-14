/*=========================================================================

File:			zoom_3d_cube.cpp
Language:		C++11
Library:		Qt 5.2.0
Author:			JUNG DAE GUN
First date:		2015-07-22
Last modify:	2015-07-24

=========================================================================*/
#include "zoom_3d_cube.h"
#include <math.h>

#include <qgraphicsitem.h>
#include <QMatrix4x4>
#include <QVector3D>
#include <QGraphicsScene>

#include "../../Common/Common/W3Memory.h"

namespace {
const float kMinZoom3DRectSize = 20.0f;
} // end of namespace

Zoom3DCube::Zoom3DCube(const QPointF &point) : center_pos_(point) {
	init();
}

Zoom3DCube::Zoom3DCube(const QPointF &point, const glm::vec3 &rightVec, const glm::vec3 &backVec) :
	center_pos_(point), m_vRightVec(rightVec), m_vBackVec(backVec) {
	init();
}

Zoom3DCube::~Zoom3DCube(void) {
	for (auto &item : cube_lines_)
		SAFE_DELETE_OBJECT(item);
	cube_lines_.clear();

	for (auto &item : center_lines_)
		SAFE_DELETE_OBJECT(item);
	center_lines_.clear();

	SAFE_DELETE_OBJECT(ellipse_);
}

void Zoom3DCube::AddToScene(QGraphicsScene *scene) {
	m_penCube.setColor(Qt::yellow);
	m_penCube.setWidthF(1.5f);
	m_penCube.setJoinStyle(Qt::MiterJoin);

	m_penEllipse.setColor(QColor(Qt::green));
	m_penEllipse.setWidthF(1.5f);
	m_penEllipse.setStyle(Qt::DotLine);

	m_penLine.setColor(Qt::red);
	m_penLine.setWidthF(1.5f);

	ellipse_ = scene->addEllipse(center_pos_.x(), center_pos_.y(), 0, 0, m_penEllipse);

	for (int i = 0; i < 12; i++)
		cube_lines_.append(scene->addLine(center_pos_.x(), center_pos_.y(), center_pos_.x(), center_pos_.y(), m_penCube));

	float fMinLineSize = kMinZoom3DRectSize / 4.0f;
	center_lines_.append(scene->addLine(center_pos_.x(), center_pos_.y(), center_pos_.x(), center_pos_.y(), m_penLine));
	center_lines_.append(scene->addLine(center_pos_.x(), center_pos_.y(), center_pos_.x(), center_pos_.y(), m_penLine));
}

void Zoom3DCube::RemoveFromScene(QGraphicsScene *scene) {
	scene->removeItem(ellipse_);
	for (auto &item : cube_lines_)
		scene->removeItem(item);
	for (auto &item : center_lines_)
		scene->removeItem(item);
}

void Zoom3DCube::resize(const float &radius) {
	resize(center_pos_, radius);
}

void Zoom3DCube::resize(const QPointF &point, const float &radius) {
	float fMinLineSize = kMinZoom3DRectSize / 4.0f;
	float fBoundingCircleHalf = 0.0f;

	if (m_bIsDrawMode) {
		center_pos_ = point;
		m_fRadius = radius;
		if (m_fRadius < kMinZoom3DRectSize)
			m_fRadius = kMinZoom3DRectSize;

		fBoundingCircleHalf = m_fRadius;

		drawCube();

		ellipse_->setRect(center_pos_.x() - fBoundingCircleHalf, center_pos_.y() - fBoundingCircleHalf,
						  fBoundingCircleHalf * 2, fBoundingCircleHalf * 2);
		center_lines_.at(0)->setLine(center_pos_.x() - fMinLineSize, center_pos_.y() - fMinLineSize,
									 center_pos_.x() + fMinLineSize, center_pos_.y() + fMinLineSize);
		center_lines_.at(1)->setLine(center_pos_.x() - fMinLineSize, center_pos_.y() + fMinLineSize,
									 center_pos_.x() + fMinLineSize, center_pos_.y() - fMinLineSize);
	} else {
		switch (m_eCurrElement) {
		case ZOOM3D_ELEMENT::SPHERE:
			m_fRadius = radius;
			if (m_fRadius < kMinZoom3DRectSize)
				m_fRadius = kMinZoom3DRectSize;

			fBoundingCircleHalf = m_fRadius;

			drawCube();
			ellipse_->setRect(center_pos_.x() - fBoundingCircleHalf, center_pos_.y() - fBoundingCircleHalf,
							  fBoundingCircleHalf * 2, fBoundingCircleHalf * 2);
			center_lines_.at(0)->setLine(center_pos_.x() - fMinLineSize, center_pos_.y() - fMinLineSize,
										 center_pos_.x() + fMinLineSize, center_pos_.y() + fMinLineSize);
			center_lines_.at(1)->setLine(center_pos_.x() - fMinLineSize, center_pos_.y() + fMinLineSize,
										 center_pos_.x() + fMinLineSize, center_pos_.y() - fMinLineSize);
			break;
		case ZOOM3D_ELEMENT::CUBE:
			fBoundingCircleHalf = m_fRadius;

			drawCube();
			ellipse_->setRect(point.x() - fBoundingCircleHalf, point.y() - fBoundingCircleHalf,
							  fBoundingCircleHalf * 2, fBoundingCircleHalf * 2);
			center_lines_.at(0)->setLine(point.x() - fMinLineSize, point.y() - fMinLineSize,
										 point.x() + fMinLineSize, point.y() + fMinLineSize);
			center_lines_.at(1)->setLine(point.x() - fMinLineSize, point.y() + fMinLineSize,
										 point.x() + fMinLineSize, point.y() - fMinLineSize);

			center_pos_ = point;
			break;
		case ZOOM3D_ELEMENT::ZOOM3D_NONE:
			break;
		default:
			break;
		}
	}
}

void Zoom3DCube::setCenter(const QPointF &center) {
	center_pos_ = center;

	float fMinLineSize = kMinZoom3DRectSize / 4.0f;
	float fBoundingCircleHalf = m_fRadius;

	drawCube();
	ellipse_->setRect(center_pos_.x() - fBoundingCircleHalf, center_pos_.y() - fBoundingCircleHalf,
					  fBoundingCircleHalf * 2, fBoundingCircleHalf * 2);
	center_lines_.at(0)->setLine(center_pos_.x() - fMinLineSize, center_pos_.y() - fMinLineSize,
								 center_pos_.x() + fMinLineSize, center_pos_.y() + fMinLineSize);
	center_lines_.at(1)->setLine(center_pos_.x() - fMinLineSize, center_pos_.y() + fMinLineSize,
								 center_pos_.x() + fMinLineSize, center_pos_.y() - fMinLineSize);
}

void Zoom3DCube::GetMargin(float& view_margin) {
	float radiusSqr = m_fRadius * m_fRadius;
	if (radiusSqr < view_margin)
		view_margin = radiusSqr * 0.5f;
}

void Zoom3DCube::setVisible(bool visible) {
	ellipse_->setVisible(visible);

	for (auto& line : center_lines_) {
		line->setVisible(visible);
	}

	for (auto& line : cube_lines_) {
		line->setVisible(visible);
	}
}

void Zoom3DCube::rotate(const glm::mat4 &rotMat, const glm::vec3 &rightVec, const glm::vec3 &backVec) {
	m_mRotMat = rotMat;
	m_vRightVec = rightVec;
	m_vBackVec = backVec;

	drawCube();
}

Zoom3DCube::ZOOM3D_ELEMENT Zoom3DCube::selectedElement(const QPointF &point) {
	if (!ellipse_)
		return ZOOM3D_ELEMENT::ZOOM3D_NONE;

	float fLineRectSize = center_lines_.at(0)->boundingRect().width();
	QRectF rect(center_pos_.x() - fLineRectSize,
				center_pos_.y() - fLineRectSize, fLineRectSize * 2, fLineRectSize * 2);

	float fDist = sqrt(pow((point.x() - center_pos_.x()), 2) +
					   pow((point.y() - center_pos_.y()), 2));

	if ((fDist <= m_fRadius + 10) && (fDist >= m_fRadius - 10)) {
		m_eCurrElement = ZOOM3D_ELEMENT::SPHERE;
	} else {
		m_eCurrElement = ZOOM3D_ELEMENT::ZOOM3D_NONE;
	}

	UpdateUI();

	return m_eCurrElement;
}

void Zoom3DCube::UpdateUI() {
	if (m_eCurrElement == ZOOM3D_ELEMENT::SPHERE) {
		QPen penHighlightEllipse = m_penEllipse;
		penHighlightEllipse.setWidthF(m_penEllipse.widthF() * 2.0f);
		ellipse_->setPen(penHighlightEllipse);
	} else {
		ellipse_->setPen(m_penEllipse);
	}
}

void Zoom3DCube::removeItemAll(QGraphicsScene *scene) {
	RemoveFromScene(scene);
}

void Zoom3DCube::addItemAll(QGraphicsScene *scene) {
	scene->addItem(ellipse_);
	for (auto &item : cube_lines_)
		scene->addItem(item);
	for (auto &item : center_lines_)
		scene->addItem(item);
}
void Zoom3DCube::transformItems(const QTransform& transform) {
	float scale = (transform.m11() + transform.m22())*0.5f;
	if (scale != 0.0f)
		m_fRadius *= scale;

	center_pos_ = transform.map(center_pos_);
	setCenter(center_pos_);
}

void Zoom3DCube::resetUI(void) {
	ellipse_->setPen(m_penEllipse);
}

void Zoom3DCube::init() {
	ellipse_ = new QGraphicsEllipseItem();
	center_lines_.clear();
	cube_lines_.clear();

	m_fRadius = 0.0f;

	m_eCurrElement = ZOOM3D_ELEMENT::ZOOM3D_NONE;

	m_bIsDrawMode = false;

	m_listPoint.clear();

	m_penCube.setColor(Qt::yellow);
	m_penCube.setWidthF(1.5f);
	m_penCube.setJoinStyle(Qt::MiterJoin);

	m_penEllipse.setColor(QColor(Qt::green));
	m_penEllipse.setWidthF(1.5f);
	m_penEllipse.setStyle(Qt::DotLine);

	m_penLine.setColor(Qt::red);
	m_penLine.setWidthF(1.5f);
}

void Zoom3DCube::drawCube() {
	float rect_half = m_fRadius / sqrt(3.0f);

	m_listPoint.clear();
	// front
	m_listPoint.append(glm::vec3(-rect_half, -rect_half, +rect_half));
	m_listPoint.append(glm::vec3(+rect_half, -rect_half, +rect_half));
	m_listPoint.append(glm::vec3(+rect_half, +rect_half, +rect_half));
	m_listPoint.append(glm::vec3(-rect_half, +rect_half, +rect_half));
	// back
	m_listPoint.append(glm::vec3(-rect_half, -rect_half, -rect_half));
	m_listPoint.append(glm::vec3(+rect_half, -rect_half, -rect_half));
	m_listPoint.append(glm::vec3(+rect_half, +rect_half, -rect_half));
	m_listPoint.append(glm::vec3(-rect_half, +rect_half, -rect_half));

	QMatrix4x4 rotMat(
		m_mRotMat[0][0], m_mRotMat[0][1], m_mRotMat[0][2], m_mRotMat[0][3],
		m_mRotMat[1][0], m_mRotMat[1][1], m_mRotMat[1][2], m_mRotMat[1][3],
		m_mRotMat[2][0], m_mRotMat[2][1], m_mRotMat[2][2], m_mRotMat[2][3],
		m_mRotMat[3][0], m_mRotMat[3][1], m_mRotMat[3][2], m_mRotMat[3][3]);

	QList<QPointF> list2DPoint;
	for (int i = 0; i < m_listPoint.size(); i++) {
		QVector3D vTempD = rotMat.mapVector(QVector3D(m_listPoint.at(i).x, m_listPoint.at(i).y, m_listPoint.at(i).z));
		glm::vec3 vD(vTempD.x(), vTempD.y(), vTempD.z());
		QPointF ptV(glm::dot(vD, m_vRightVec) + center_pos_.x(), glm::dot(vD, m_vBackVec) + center_pos_.y());
		list2DPoint.append(ptV);
	}

	cube_lines_.at(0)->setLine(list2DPoint.at(0).x(), list2DPoint.at(0).y(), list2DPoint.at(1).x(), list2DPoint.at(1).y());
	cube_lines_.at(1)->setLine(list2DPoint.at(1).x(), list2DPoint.at(1).y(), list2DPoint.at(2).x(), list2DPoint.at(2).y());
	cube_lines_.at(2)->setLine(list2DPoint.at(2).x(), list2DPoint.at(2).y(), list2DPoint.at(3).x(), list2DPoint.at(3).y());
	cube_lines_.at(3)->setLine(list2DPoint.at(3).x(), list2DPoint.at(3).y(), list2DPoint.at(0).x(), list2DPoint.at(0).y());

	cube_lines_.at(4)->setLine(list2DPoint.at(4).x(), list2DPoint.at(4).y(), list2DPoint.at(5).x(), list2DPoint.at(5).y());
	cube_lines_.at(5)->setLine(list2DPoint.at(5).x(), list2DPoint.at(5).y(), list2DPoint.at(6).x(), list2DPoint.at(6).y());
	cube_lines_.at(6)->setLine(list2DPoint.at(6).x(), list2DPoint.at(6).y(), list2DPoint.at(7).x(), list2DPoint.at(7).y());
	cube_lines_.at(7)->setLine(list2DPoint.at(7).x(), list2DPoint.at(7).y(), list2DPoint.at(4).x(), list2DPoint.at(4).y());

	cube_lines_.at(8)->setLine(list2DPoint.at(0).x(), list2DPoint.at(0).y(), list2DPoint.at(4).x(), list2DPoint.at(4).y());
	cube_lines_.at(9)->setLine(list2DPoint.at(1).x(), list2DPoint.at(1).y(), list2DPoint.at(5).x(), list2DPoint.at(5).y());
	cube_lines_.at(10)->setLine(list2DPoint.at(2).x(), list2DPoint.at(2).y(), list2DPoint.at(6).x(), list2DPoint.at(6).y());
	cube_lines_.at(11)->setLine(list2DPoint.at(3).x(), list2DPoint.at(3).y(), list2DPoint.at(7).x(), list2DPoint.at(7).y());
}
