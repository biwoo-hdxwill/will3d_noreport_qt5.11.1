#include "W3SurfaceTextEllipseItem.h"

#include <exception>
#include <iostream>

#include <QGraphicsScene>
#include <QApplication>

#include "../../Common/Common/W3Memory.h"
#include "../../UIModule/UIPrimitive/W3TextItem.h"

using namespace std;

CW3SurfaceTextEllipseItem::CW3SurfaceTextEllipseItem(QGraphicsScene* pScene)
	: m_pgScene(pScene) {}
CW3SurfaceTextEllipseItem::~CW3SurfaceTextEllipseItem() {
	this->clear();
}

void CW3SurfaceTextEllipseItem::clear() {
	while (m_lstText.size()) {
		auto iter = m_lstText.begin();
		m_pgScene->removeItem(*iter);
		SAFE_DELETE_OBJECT(*iter);
		m_lstText.erase(iter);
	}
	m_isVisible.clear();
	m_mapPosition.clear();

	CW3SurfaceEllipseItem::clear();
}
void CW3SurfaceTextEllipseItem::addItem(const QString& text, const glm::vec3& point) {
	if (m_mapPosition.find(text) == m_mapPosition.end()) {
		CW3TextItem* txtLandmark = new CW3TextItem(QApplication::font(), text, "#FF82ABFF");
		txtLandmark->setBackground("#72303030");
		txtLandmark->setTextBold(true);
		m_pgScene->addItem(txtLandmark);
		m_lstText.push_back(txtLandmark);
		m_mapPosition[text] = point;
		CW3SurfaceEllipseItem::addPoint(point);
	} else
		this->editItem(text, point);

	m_isVisible[text] = true;
}
void CW3SurfaceTextEllipseItem::editItem(const QString & text, const glm::vec3 & point) {
	try {
		if (m_mapPosition.find(text) == m_mapPosition.end())
			throw runtime_error("item isn't exist");

		for (auto& elem : m_points) {
			if (elem == m_mapPosition[text]) {
				elem = point;
				break;
			}
		}

		m_mapPosition[text] = point;
	} catch (const runtime_error& e) {
		cout << "CW3SurfaceTextEllipseItem::editItem: " << e.what() << endl;
	}
}
void CW3SurfaceTextEllipseItem::editItems(const std::map<QString, glm::vec3> points) {
	for (const auto& elem : points)
		this->editItem(elem.first, elem.second);
}
void CW3SurfaceTextEllipseItem::eraseItem(const QString& text) {
	for (int i = (int)m_lstText.size() - 1; i >= 0; i--) {
		if (m_lstText[i]->toPlainText() == text) {
			m_pgScene->removeItem(m_lstText[i]);
			SAFE_DELETE_OBJECT(m_lstText[i]);
			m_lstText.erase(m_lstText.begin() + i);
			CW3SurfaceEllipseItem::erasePoint(i);
			m_isVisible.erase(text);
			break;
		}
	}
}
void CW3SurfaceTextEllipseItem::setSceneSizeInView(int width, int height) {
	m_sceneWinView = width;
	m_sceneHinView = height;
}
void CW3SurfaceTextEllipseItem::draw(GLuint program) {
	for (int i = 0; i < m_lstText.size(); i++) {
		if (!m_isVisible[m_lstText[i]->toPlainText()]) {
			m_lstText[i]->setVisible(false);
			continue;
		}

		CW3SurfaceEllipseItem::draw(i, program);

		m_lstText[i]->setVisible(true);

		glm::vec3 pos = m_points[i];

		glm::mat4 mvp = m_projection * m_view*m_transform.arcball*m_transform.reorien*m_transform.scale;
		glm::vec4 ptScreen = mvp * vec4(pos, 1.0f);
		QPointF ptScene = QPointF(ptScreen.x*m_sceneWinView, (ptScreen.y*-1.0f)*m_sceneHinView) + QPointF(m_sceneWinView, m_sceneHinView);

		m_lstText[i]->setPos(ptScene);
	}
}
void CW3SurfaceTextEllipseItem::draw(const QString& text, GLuint program) {
	int index = -1;
	for (int i = 0; i < m_lstText.size(); i++) {
		if (m_lstText[i]->toPlainText() == text) {
			m_lstText[i]->setVisible(true);
			index = i;
			break;
		} else {
			m_lstText[i]->setVisible(false);
		}
	}

	if (index < 0)
		return;

	CW3SurfaceEllipseItem::draw(index, program);

	glm::vec3 pos = m_points[index];
	glm::mat4 mvp = m_projection * m_view*m_transform.arcball*m_transform.reorien*m_transform.scale;
	glm::vec4 ptScreen = mvp * vec4(pos, 1.0f);
	QPointF ptScene = QPointF(ptScreen.x*m_sceneWinView, (ptScreen.y*-1.0f)*m_sceneHinView) + QPointF(m_sceneWinView, m_sceneHinView);

	m_lstText[index]->setPos(ptScene);
}
void CW3SurfaceTextEllipseItem::draw(const QStringList& texts, GLuint program) {
	std::vector<int> indices;
	for (int i = 0; i < m_lstText.size(); i++) {
		for (int j = 0; j < texts.size(); j++) {
			if (m_lstText[i]->toPlainText() == texts[j]) {
				m_lstText[i]->setVisible(true);
				indices.push_back(i);
				break;
			} else {
				m_lstText[i]->setVisible(false);
			}
		}
	}

	if (indices.size() == 0)
		return;

	for (int i = 0; i < indices.size(); i++) {
		CW3SurfaceEllipseItem::draw(indices[i], program);

		glm::mat4 mvp = m_projection * m_view*m_transform.arcball*m_transform.reorien*m_transform.scale;
		glm::vec3 pos = m_points[indices[i]];

		glm::vec4 ptScreen = mvp * vec4(pos, 1.0f);
		QPointF ptScene = QPointF(ptScreen.x*m_sceneWinView, (ptScreen.y*-1.0f)*m_sceneHinView) + QPointF(m_sceneWinView, m_sceneHinView);

		m_lstText[indices[i]]->setPos(ptScene);
	}
}
