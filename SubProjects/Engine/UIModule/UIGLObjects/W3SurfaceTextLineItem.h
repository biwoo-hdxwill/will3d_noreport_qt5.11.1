#pragma once

/*=========================================================================

File:			class CW3SurfaceTextLineItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-11-15
Last modify:	2016-11-15

=========================================================================*/
#include "W3SurfaceLineItem.h"

#include <QObject>

class QGraphicsScene;
class CW3TextItem;
class CW3LabelItem_anno;
class CW3PathItem;

class UIGLOBJECTS_EXPORT CW3SurfaceTextLineItem : public QObject, public CW3SurfaceLineItem
{
	Q_OBJECT

public:
	enum class LabelPos
	{
		FRONT,
		CENTER,
		BACK
	};

	CW3SurfaceTextLineItem(QGraphicsScene* pScene, SHAPE shape = CURVE, bool bDrawEllipse = true);
	~CW3SurfaceTextLineItem();

public:
	void setText(const QString& text);

	virtual void draw(GLuint program);
	virtual void clear() override;

	virtual void SetVisible(const bool visible) override;

	inline void setSceneSizeInView(int nSceneWinView, int nSceneHinView) { 
		m_nSceneWinView = nSceneWinView;
		m_nSceneHinView = nSceneHinView;
	}

	inline const bool label_selected() const { return label_selected_; }

	void SetLabelPos(LabelPos pos);

	virtual void ApplyPreferences() override;

protected:
	virtual void createLine() override;

protected:
	glm::vec3 m_textGLpos;

private:
	void DrawingLabel();
	QPointF MapGLVertexToScene(glm::vec3 gl_vertex_pos);

	void ApplyTextColor();
	void ApplyTextSize();

private slots:
	void slotLabelTranslated(QPointF trans);
	void slotLabelHoverd(bool hovered);

private:
	QGraphicsScene*		m_pgScene;
	//drawing distance text
	bool				m_bDrawText = false;
	int				m_nSceneWinView = 0;
	int				m_nSceneHinView = 0;
	//CW3TextItem*		m_text = nullptr;

	CW3LabelItem_anno* label_ = nullptr;
	CW3PathItem* guide_line_ = nullptr;
	QPointF label_position_offset_;
	bool label_selected_ = false;

	GLuint program_ = 0;

	LabelPos label_pos_ = LabelPos::CENTER;
};
