#include "W3SurfaceTextLineItem.h"

#include <QGraphicsScene>
#include <QApplication>
#include <QWidget>

#include "../../Common/Common/W3Memory.h"
#include "../UIPrimitive/W3TextItem.h"
#include <Engine/UIModule/UIPrimitive/W3LabelItem_anno.h>
#include <Engine/UIModule/UIPrimitive/W3PathItem.h>
#include <Engine/Common/Common/global_preferences.h>

using namespace std;
using glm::vec3;

CW3SurfaceTextLineItem::CW3SurfaceTextLineItem(QGraphicsScene* pScene, SHAPE shape, bool bDrawEllipse)
	: m_pgScene(pScene), CW3SurfaceLineItem(shape, bDrawEllipse)
{
}
CW3SurfaceTextLineItem::~CW3SurfaceTextLineItem()
{
	/*if (m_text)
	{
		m_pgScene->removeItem(m_text);
		SAFE_DELETE_OBJECT(m_text);
	}*/

	if (label_)
	{
		m_pgScene->removeItem(label_);
		SAFE_DELETE_OBJECT(label_);
	}

	if (guide_line_)
	{
		m_pgScene->removeItem(guide_line_);
		SAFE_DELETE_OBJECT(guide_line_);
	}
}

void CW3SurfaceTextLineItem::setText(const QString& text)
{
	/*if (!m_text)
	{
		m_text = new CW3TextItem();
		m_text->setTextColor("#FF82ABFF");
		m_text->setBackground("#72303030");
		m_text->setHoverEnabled(false);
		m_text->setTextBold(true);
		m_pgScene->addItem(m_text);
	}

	m_text->setPlainText(text);*/

	if (!label_)
	{
		label_ = new CW3LabelItem_anno();
		label_->setZValue(1);
		connect(label_, SIGNAL(sigTranslateLabel(QPointF)), this, SLOT(slotLabelTranslated(QPointF)));
		connect(label_, SIGNAL(sigHoverLabel(bool)), this, SLOT(slotLabelHoverd(bool)));
		//connect(label_, SIGNAL(sigPressed()), this, SLOT(slotLabelClicked()));

		ApplyTextColor();
		ApplyTextSize();
		m_pgScene->addItem(label_);

		guide_line_ = new CW3PathItem();
		guide_line_->setZValue(0);
		guide_line_->setLineStatic(true);
		guide_line_->setMovable(false);
		guide_line_->setPen(QPen(Qt::yellow, 2, Qt::DotLine, Qt::FlatCap));
		m_pgScene->addItem(guide_line_);

		label_position_offset_ = QPointF(30, -30);
	}

	if (text.compare(label_->toPlainText()))
	{
		label_->setPlainText(text);
	}

	m_bDrawText = true;

	DrawingLabel();
}

void CW3SurfaceTextLineItem::draw(GLuint program)
{
	if (!visible_)
	{
		return;
	}

	program_ = program;

	CW3SurfaceLineItem::draw(program);

#if 0
	if (m_bDrawText && m_text != nullptr)
	{
		glm::mat4 mvp = m_projection*m_view*m_transform.arcball*m_transform.scale;

		glm::vec4 ptScreen = mvp*vec4(m_textGLpos, 1.0f);

		QPointF ptScene = QPointF(ptScreen.x*m_nSceneWinView, (ptScreen.y*-1.0f)*m_nSceneHinView) + QPointF(m_nSceneWinView, m_nSceneHinView);

		m_text->setPos(ptScene);
	}
#else
	DrawingLabel();
#endif
}

void CW3SurfaceTextLineItem::createLine()
{
	if (m_points.size() < 2)
	{
		return;
	}

	CW3SurfaceLineItem::createLine();

#if 0
	if (m_bDrawText)
	{
#if 0
		vec3 txtPos = vec3(0.0f);

		for (int i = 0; i < m_points.size(); i++)
		{
			txtPos += m_points[i];
		}
		txtPos /= m_points.size();

		m_textGLpos = txtPos;
#else
		m_textGLpos = m_points.at(m_points.size() - 1);
#endif
	}
#endif
}

void CW3SurfaceTextLineItem::clear()
{
	CW3SurfaceLineItem::clear();

	/*if (m_text)
	{
		m_pgScene->removeItem(m_text);
		SAFE_DELETE_OBJECT(m_text);
	}*/

	if (label_)
	{
		m_pgScene->removeItem(label_);
		SAFE_DELETE_OBJECT(label_);
	}

	if (guide_line_)
	{
		m_pgScene->removeItem(guide_line_);
		SAFE_DELETE_OBJECT(guide_line_);
	}

	clearVAOVBO();
}

void CW3SurfaceTextLineItem::SetVisible(const bool visible)
{
	CW3SurfaceLineItem::SetVisible(visible);

	/*if (m_text)
	{
		m_text->setVisible(visible);
	}*/

	if (label_)
	{
		label_->setVisible(visible);
	}

	if (guide_line_)
	{
		guide_line_->setVisible(visible);
	}
}

void CW3SurfaceTextLineItem::slotLabelTranslated(QPointF trans)
{
	label_position_offset_ += trans;

	DrawingLabel();
}

void CW3SurfaceTextLineItem::slotLabelHoverd(bool hovered)
{
	SetSelected(hovered);

	label_selected_ = hovered;
	label_->setSelected(hovered);
}

void CW3SurfaceTextLineItem::DrawingLabel()
{
	if (!m_bDrawText || !guide_line_ || !label_ || m_points.size() < 2)
	{
		return;
	}

	QPointF st, et, mt;
#if 0
	st = MapGLVertexToScene(m_points.at(0));
	et = MapGLVertexToScene(m_points.at(1));

	if (st.rx() < et.rx())
		mt.setX(st.rx() + fabs(st.rx() - et.rx()) / 2);
	else
		mt.setX(et.rx() + fabs(et.rx() - st.rx()) / 2);

	if (st.ry() < et.ry())
		mt.setY(st.ry() + fabs(st.ry() - et.ry()) / 2);
	else
		mt.setY(et.ry() + fabs(et.ry() - st.ry()) / 2);
#else

	switch (label_pos_)
	{
	case LabelPos::FRONT:
		mt = MapGLVertexToScene(m_points.at(0));
		break;
	case LabelPos::CENTER:
		switch (m_points.size())
		{
		case 2:
		{
			QPointF sum;
			for (int i = 0; i < m_points.size(); ++i)
			{
				sum += MapGLVertexToScene(m_points.at(i));
			}
			mt = sum / m_points.size();
		}
		break;
		case 3:
			mt = MapGLVertexToScene(m_points.at(1));
			break;
		default:
			break;
		}
		break;
	case LabelPos::BACK:
		mt = MapGLVertexToScene(m_points.at(m_points.size() - 1));
		break;
	default:
		break;
	}

#endif

	st = mt;
	et = mt + label_position_offset_;

	QPointF label_position(et.rx(), et.ry() - label_->boundingRect().height());

	if (et.x() < 0)
	{
		et.setX(0);
	}
	else if (((QWidget*)m_pgScene->parent())->width() < (et.x() + label_->boundingRect().width()))
	{
		et.setX(((QWidget*)m_pgScene->parent())->width() - label_->boundingRect().width());
	}

	if (et.y() < label_->boundingRect().height())
	{
		et.setY(label_->boundingRect().height());
	}
	else if (((QWidget*)m_pgScene->parent())->height() < et.y())
	{
		et.setY(((QWidget*)m_pgScene->parent())->height());
	}

	if (label_position.x() < 0)
	{
		label_position.setX(0);
	}
	else if (((QWidget*)m_pgScene->parent())->width() < (label_position.x() + label_->boundingRect().width()))
	{
		label_position.setX(((QWidget*)m_pgScene->parent())->width() - label_->boundingRect().width());
	}

	if (label_position.y() < 0)
	{
		label_position.setY(0);
	}
	else if (((QWidget*)m_pgScene->parent())->height() < (label_position.y() + label_->boundingRect().height()))
	{
		label_position.setY(((QWidget*)m_pgScene->parent())->height() - label_->boundingRect().height());
	}

	QPainterPath p(st);
	p.lineTo(et);
	guide_line_->setPath(p);

	if (label_position != label_->pos())
	{
		label_->setPos(label_position);
		label_position_offset_ = QPointF((label_position - st).x(), (label_position - st).y() + label_->boundingRect().height());
	}
}

QPointF CW3SurfaceTextLineItem::MapGLVertexToScene(glm::vec3 gl_vertex_pos)
{
	glm::mat4 mvp = m_projection * m_view * m_transform.arcball * m_transform.reorien * m_transform.scale;
	glm::vec4 screen_position = mvp * vec4(gl_vertex_pos, 1.0f);
	screen_position /= screen_position.w;
	QPointF scene_position = QPointF(screen_position.x * m_nSceneWinView, (screen_position.y * -1.0f) * m_nSceneHinView) + QPointF(m_nSceneWinView, m_nSceneHinView);

	return scene_position;
}

void CW3SurfaceTextLineItem::SetLabelPos(LabelPos pos)
{
	label_pos_ = pos;

	DrawingLabel();
}

void CW3SurfaceTextLineItem::ApplyPreferences()
{
	CW3SurfaceLineItem::ApplyPreferences();

	ApplyTextColor();
	ApplyTextSize();
}

void CW3SurfaceTextLineItem::ApplyTextColor()
{
	if (!label_)
	{
		return;
	}
	QColor text_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.text_color;
	label_->SetTextColor(text_color);
}

void CW3SurfaceTextLineItem::ApplyTextSize()
{
	if (!label_)
	{
		return;
	}
	GlobalPreferences::Size text_size = GlobalPreferences::GetInstance()->preferences_.objects.measure.text_size;
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() + (static_cast<int>(text_size) * 2) + 1);
	label_->setFont(font);
}
