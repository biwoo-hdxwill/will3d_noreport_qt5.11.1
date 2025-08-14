#pragma once

/*=========================================================================

File:			class CW3SurfaceTextEllipseItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-05-17
Last modify:	2016-05-17

=========================================================================*/
#include <map>
#include <qstring.h>
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "W3SurfaceEllipseItem.h"

class QGraphicsScene;
class CW3TextItem;

class UIGLOBJECTS_EXPORT CW3SurfaceTextEllipseItem : public CW3SurfaceEllipseItem
{
public:
	CW3SurfaceTextEllipseItem(QGraphicsScene* pScene);
	~CW3SurfaceTextEllipseItem();
	 
public:
	void clear();

	void draw(GLuint program) override;
	void draw(const QString& text, GLuint program);
	void draw(const QStringList& texts, GLuint program);
	void addItem(const QString& text, const glm::vec3& point);
	void editItem(const QString& text, const glm::vec3& point);
	void editItems(const std::map<QString, glm::vec3> points);
	void eraseItem(const QString& text);
	void setSceneSizeInView(int width, int height);

	inline void setVisibleItem(const QString& text, bool isVisible) {
		if (m_isVisible.find(text) != m_isVisible.end())
			m_isVisible[text] = isVisible;
	}
	inline void setVisibleItems(bool isVisible)
	{
		for (auto& elem : m_isVisible)
			elem.second = isVisible;
	}

	inline const std::map<QString, glm::vec3>& getPositions() const { return m_mapPosition; }
	inline const QList<CW3TextItem*>& getTextList() const { return m_lstText; }

private:

	QGraphicsScene* m_pgScene;

	QList<CW3TextItem*> m_lstText;
	int m_sceneWinView;
	int m_sceneHinView;

	std::map<QString, glm::vec3> m_mapPosition;
	std::map<QString, bool> m_isVisible;
};
