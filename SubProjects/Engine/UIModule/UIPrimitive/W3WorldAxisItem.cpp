#include "W3WorldAxisItem.h"

#if defined(__APPLE__)
#include <glm/gtx/transform2.hpp>
#else
#include <GL/glm/gtx/transform2.hpp>
#endif

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <qfont.h>
#include <qpen.h>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3Memory.h"

CW3WorldAxisItem::CW3WorldAxisItem(bool is2D, bool isPano)
	: m_is2D(is2D) {
	this->setAcceptHoverEvents(true);
	this->setFlag(QGraphicsItem::ItemIsSelectable, false); //170801. 임시코드.
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);

	m_isHovered = false;

	for (int i = 0; i < 3; i++) {
		QGraphicsLineItem* line = new QGraphicsLineItem(this);
		QGraphicsTextItem* text = new QGraphicsTextItem(this);
		text->setFont(font);
		m_lines.push_back(line);
		m_texts.push_back(text);
	}

	m_lines[X]->pen().setCosmetic(true);
	m_lines[Y]->pen().setCosmetic(true);
	m_lines[Z]->pen().setCosmetic(true);

	m_scaleMatrix = glm::scale(glm::vec3(1.0f, 1.0f, -1.0f));
	m_scaleMatrix3D = glm::scale(glm::vec3(1.0f, -1.0f, 1.0f));
	m_ViewPlaneMatrix = glm::mat4(1.0f);

	m_dir = glm::mat4(1.0f);
	m_dirLast = glm::mat4(1.0f);
	m_bases = glm::mat4(1.0f);

	if (is2D) {
		if (isPano) {
			m_dir[X] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);	// right
			m_dir[Y] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);	// front
			m_dir[Z] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);	// bottom
		} else {
			m_dir[X] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);	// right
			m_dir[Y] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);	// front
			m_dir[Z] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);	// bottom
		}
	} else {
		if (isPano) {
			m_dir[X] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);	// right
			m_dir[Y] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);	// front
			m_dir[Z] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);	// bottom
		} else {
			m_dir[X] = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);	// right
			m_dir[Y] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);	// front
			m_dir[Z] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);	// bottom
		}
	}

	m_texts[X]->setPlainText("left");
	m_texts[Y]->setPlainText("front");
	m_texts[Z]->setPlainText("bottom");

	m_opacityEllipse = new QGraphicsEllipseItem(this);
	m_opacityEllipse->setPos(0, 0);
	QPen opacityPen = m_opacityEllipse->pen();
	opacityPen.setCosmetic(true);
	opacityPen.setWidth(0);
	m_opacityEllipse->setPen(opacityPen);
	m_opacityEllipse->setOpacity(0.8f);

	m_centerEllipse = new QGraphicsEllipseItem(this);
	m_centerEllipse->setPos(0, 0);
	QPen centerPen = m_centerEllipse->pen();
	centerPen.setCosmetic(true);
	centerPen.setWidth(0);
	m_centerEllipse->setPen(centerPen);

	m_centerEllipse->setZValue(0.0f);
	m_opacityEllipse->setZValue(0.0f);

	this->setSize(45);

	setColor();
	m_isDarkMode = false;
}

CW3WorldAxisItem::~CW3WorldAxisItem() {
	for (auto elem : m_texts)
		SAFE_DELETE_OBJECT(elem);
	for (auto elem : m_lines)
		SAFE_DELETE_OBJECT(elem);

	SAFE_DELETE_OBJECT(m_opacityEllipse);
	SAFE_DELETE_OBJECT(m_centerEllipse);
}

//////////////////////////////////////////////////////////////////////////////////////
// public functions
//////////////////////////////////////////////////////////////////////////////////////
void CW3WorldAxisItem::setSize(const int size) {
	m_length = size;
	m_centerEllipse->setRect(-size * 0.15, -size * 0.15, size*0.3, size*0.3);

	m_opacityEllipse->setRect(-size * 0.7f, -size * 0.7f, size*1.4f, size*1.4f);
	m_rect = QRectF(-size, -size, size*2.0f, size*2.0f);

	m_dirLast[X] = m_bases[X] = m_dir[X] = m_dir[X] * m_length;
	m_dirLast[Y] = m_bases[Y] = m_dir[Y] = m_dir[Y] * m_length;
	m_dirLast[Z] = m_bases[Z] = m_dir[Z] = m_dir[Z] * m_length;

	m_basesInverse = glm::inverse(m_bases);

	for (int i = 0; i < 3; i++) {
		QGraphicsLineItem* line = m_lines.at(i);
		QPen pen = line->pen();
		pen.setWidthF(m_length * 0.07f);
		line->setPen(pen);
	}
}

void CW3WorldAxisItem::setDirection(const glm::mat4 &cameraMat) {
	m_dirLast = m_scaleMatrix3D * cameraMat*m_bases;

	m_lines[X]->setLine(m_dirLast[X].x*0.1f, m_dirLast[X].y*0.1f, m_dirLast[X].x + 1e-5, m_dirLast[X].y);
	m_texts[X]->setPos(m_dirLast[X].x*1.2f, m_dirLast[X].y*1.2f);
	m_lines[X]->setZValue(m_dirLast[X].z);
	m_texts[X]->setZValue(m_dirLast[X].z);

	m_lines[Y]->setLine(m_dirLast[Y].x*0.1f, m_dirLast[Y].y*0.1f, m_dirLast[Y].x + 1e-5, m_dirLast[Y].y);
	m_texts[Y]->setPos(m_dirLast[Y].x*1.2f, m_dirLast[Y].y*1.2f);
	m_lines[Y]->setZValue(m_dirLast[Y].z);
	m_texts[Y]->setZValue(m_dirLast[Y].z);

	m_lines[Z]->setLine(m_dirLast[Z].x*0.1f, m_dirLast[Z].y*0.1f, m_dirLast[Z].x + 1e-5, m_dirLast[Z].y);
	m_texts[Z]->setPos(m_dirLast[Z].x*1.2f, m_dirLast[Z].y*1.2f);
	m_lines[Z]->setZValue(m_dirLast[Z].z);
	m_texts[Z]->setZValue(m_dirLast[Z].z);
}
void CW3WorldAxisItem::setDirection(const glm::mat4 &rotateMat, const glm::mat4& viewMat) {
	m_viewMatrix = viewMat;
	this->setDirection(viewMat*rotateMat);
}

void CW3WorldAxisItem::setVectors(const glm::vec3 &right_, const glm::vec3 &bottom_, const glm::vec3 &up_) {
	m_ViewPlaneMatrix[0] = glm::vec4(right_.x, bottom_.x, up_.x, 0.0f);
	m_ViewPlaneMatrix[1] = glm::vec4(right_.y, bottom_.y, up_.y, 0.0f);
	m_ViewPlaneMatrix[2] = glm::vec4(right_.z, bottom_.z, up_.z, 0.0f);

	m_dirLast = m_scaleMatrix * m_ViewPlaneMatrix * m_bases;

	m_lines[X]->setLine(m_dirLast[X].x*0.1f, m_dirLast[X].y*0.1f, m_dirLast[X].x + 1e-5, m_dirLast[X].y);
	m_texts[X]->setPos(m_dirLast[X].x*1.2f, m_dirLast[X].y*1.2f);
	m_lines[X]->setZValue(m_dirLast[X].z);
	m_texts[X]->setZValue(m_dirLast[X].z);

	m_lines[Y]->setLine(m_dirLast[Y].x*0.1f, m_dirLast[Y].y*0.1f, m_dirLast[Y].x + 1e-5, m_dirLast[Y].y);
	m_texts[Y]->setPos(m_dirLast[Y].x*1.2f, m_dirLast[Y].y*1.2f);
	m_lines[Y]->setZValue(m_dirLast[Y].z);
	m_texts[Y]->setZValue(m_dirLast[Y].z);

	m_lines[Z]->setLine(m_dirLast[Z].x*0.1f, m_dirLast[Z].y*0.1f, m_dirLast[Z].x + 1e-5, m_dirLast[Z].y);
	m_texts[Z]->setPos(m_dirLast[Z].x*1.2f, m_dirLast[Z].y*1.2f);
	m_lines[Z]->setZValue(m_dirLast[Z].z);
	m_texts[Z]->setZValue(m_dirLast[Z].z);
}

void CW3WorldAxisItem::setBases(glm::mat4 *rotate) {
	m_bases = *rotate*m_dir;

	m_basesInverse = glm::inverse(m_bases);
}

void CW3WorldAxisItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsItem::hoverEnterEvent(event);
	m_isHovered = true;
}

void CW3WorldAxisItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsItem::hoverLeaveEvent(event);
	m_isHovered = false;
}

void CW3WorldAxisItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	QGraphicsItem::mouseMoveEvent(event);
	if (event->buttons() == Qt::LeftButton) {
		glm::mat4 T;
		bool isRotated = this->rotateCompass(T, event->scenePos(), event->lastScenePos());

		if (!isRotated)
			return;

		glm::mat4 rotate = glm::inverse(m_viewMatrix)*T;
		rotate[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		emit sigRotateMatrix(rotate);
	}
}

bool CW3WorldAxisItem::rotateCompass(glm::mat4& T,
									 const QPointF &curPos, const QPointF &lastPos) {
	printf("%f, %f / %f, %f\r\n", curPos.x(), curPos.y(), lastPos.x(), lastPos.y());

	QPointF centerPos = this->pos();

	QPointF cur = curPos - centerPos;
	cur = cur / m_length;
	QPointF last = lastPos - centerPos;
	last = last / m_length;

	glm::vec3 curV = ArcBallVector(cur);
	glm::vec3 lastV = ArcBallVector(last);

	if (glm::length(curV - lastV) > 1e-5f) {
		float rotAngle = std::acos(std::min(1.0f, glm::dot(curV, lastV)));
		glm::vec3 rotAxis = glm::cross(lastV, curV);
		rotAxis = glm::normalize(rotAxis);
		m_dirLast = glm::rotate(rotAngle, rotAxis) * m_dirLast;

		m_lines[X]->setLine(m_dirLast[X].x*0.1f, m_dirLast[X].y*0.1f, m_dirLast[X].x + 1e-5, m_dirLast[X].y);
		m_texts[X]->setPos(m_dirLast[X].x*1.2f, m_dirLast[X].y*1.2f);
		m_lines[X]->setZValue(m_dirLast[X].z);
		m_texts[X]->setZValue(m_dirLast[X].z);

		m_lines[Y]->setLine(m_dirLast[Y].x*0.1f, m_dirLast[Y].y*0.1f, m_dirLast[Y].x + 1e-5, m_dirLast[Y].y);
		m_texts[Y]->setPos(m_dirLast[Y].x*1.2f, m_dirLast[Y].y*1.2f);
		m_lines[Y]->setZValue(m_dirLast[Y].z);
		m_texts[Y]->setZValue(m_dirLast[Y].z);

		m_lines[Z]->setLine(m_dirLast[Z].x*0.1f, m_dirLast[Z].y*0.1f, m_dirLast[Z].x + 1e-5, m_dirLast[Z].y);
		m_texts[Z]->setPos(m_dirLast[Z].x*1.2f, m_dirLast[Z].y*1.2f);
		m_lines[Z]->setZValue(m_dirLast[Z].z);
		m_texts[Z]->setZValue(m_dirLast[Z].z);

		if (m_is2D) {
			T = glm::transpose(m_scaleMatrix*m_dirLast*m_basesInverse);
		} else {
			T = m_scaleMatrix3D * m_dirLast*m_basesInverse;
		}

		return true;
	} else {
		T = glm::mat4(1.0f);

		return false;
	}
}

glm::vec3 CW3WorldAxisItem::ArcBallVector(QPointF &v) {
	glm::vec3 ABvector = glm::vec3(v.x(), v.y(), 0.0f);

	float xySq = ABvector.x*ABvector.x + ABvector.y*ABvector.y;

	if (xySq < 1.0f) {
		ABvector.z = sqrt(1.0f - xySq);
	} else {
		ABvector = glm::normalize(ABvector);
	}

	return ABvector;
}

void CW3WorldAxisItem::setDarkMode(bool isDark) {
	if (m_isDarkMode == isDark)
		return;

	m_isDarkMode = isDark;

	if (m_isDarkMode) {
		QPen pen;

		pen = m_lines[X]->pen();
		pen.setColor(QColor(pen.color().red() * 0.5f, pen.color().green() * 0.5f, pen.color().blue() * 0.5f));
		m_lines[X]->setPen(pen);

		pen = m_lines[Y]->pen();
		pen.setColor(QColor(pen.color().red() * 0.5f, pen.color().green() * 0.5f, pen.color().blue() * 0.5f));
		m_lines[Y]->setPen(pen);

		pen = m_lines[Z]->pen();
		pen.setColor(QColor(pen.color().red() * 0.5f, pen.color().green() * 0.5f, pen.color().blue() * 0.5f));
		m_lines[Z]->setPen(pen);

		QColor colorText;

		colorText = m_texts[X]->defaultTextColor();
		colorText = QColor(colorText.red() * 0.5f, colorText.green() * 0.5f, colorText.blue() * 0.5f);
		m_texts[X]->setDefaultTextColor(colorText);

		colorText = m_texts[Y]->defaultTextColor();
		colorText = QColor(colorText.red() * 0.5f, colorText.green() * 0.5f, colorText.blue() * 0.5f);
		m_texts[Y]->setDefaultTextColor(colorText);

		colorText = m_texts[Z]->defaultTextColor();
		colorText = QColor(colorText.red() * 0.5f, colorText.green() * 0.5f, colorText.blue() * 0.5f);
		m_texts[Z]->setDefaultTextColor(colorText);

		QBrush brushEllipse;

		pen = m_opacityEllipse->pen();
		pen.setColor(QColor(pen.color().red() * 0.5f, pen.color().green() * 0.5f, pen.color().blue() * 0.5f));
		m_opacityEllipse->setPen(pen);
		brushEllipse = m_opacityEllipse->brush();
		brushEllipse.setColor(QColor(brushEllipse.color().red() * 0.5f, brushEllipse.color().green() * 0.5f, brushEllipse.color().blue() * 0.5f));
		m_opacityEllipse->setBrush(brushEllipse);

		pen = m_centerEllipse->pen();
		pen.setColor(QColor(pen.color().red() * 0.5f, pen.color().green() * 0.5f, pen.color().blue() * 0.5f));
		m_centerEllipse->setPen(pen);
		brushEllipse = m_centerEllipse->brush();
		brushEllipse.setColor(QColor(brushEllipse.color().red() * 0.5f, brushEllipse.color().green() * 0.5f, brushEllipse.color().blue() * 0.5f));
		m_centerEllipse->setBrush(brushEllipse);
	} else {
		setColor();
	}
}

void CW3WorldAxisItem::setColor() {
	m_lines[X]->setPen(QPen(AxisColor::kX, m_length * 0.1f, Qt::SolidLine));
	m_lines[Y]->setPen(QPen(AxisColor::kY, m_length * 0.1f, Qt::SolidLine));
	m_lines[Z]->setPen(QPen(AxisColor::kZ, m_length * 0.1f, Qt::SolidLine));

	m_texts[X]->setDefaultTextColor(AxisColor::kX);
	m_texts[Y]->setDefaultTextColor(AxisColor::kY);
	m_texts[Z]->setDefaultTextColor(AxisColor::kZ);

	if (m_opacityEllipse) {
		m_opacityEllipse->setPen(QPen(QColor(96, 96, 96), 0, Qt::SolidLine));
		m_opacityEllipse->setBrush(QBrush(QColor(96, 96, 96)));
	}

	if (m_centerEllipse) {
		m_centerEllipse->setPen(QPen(Qt::white, 0, Qt::SolidLine));
		m_centerEllipse->setBrush(QBrush(Qt::white));
	}
}
