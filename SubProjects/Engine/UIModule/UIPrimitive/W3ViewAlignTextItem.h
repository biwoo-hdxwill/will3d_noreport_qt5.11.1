#pragma once

/**=================================================================================================

Project: 			UIPrimitive
File:				W3ViewAlignTextItem.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-31
Last modify:		2017-07-31

 *===============================================================================================**/
#include <vector>

#include <QObject>
#include <QGraphicsItem>
#include <QRectF>

#if defined(__APPLE__)
#include <glm/gtx/transform2.hpp>
#include <glm/detail/type_mat4x4.hpp>
#else
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/detail/type_mat4x4.hpp>
#endif

#include "uiprimitive_global.h"
class CW3TextItem;

namespace UIPrimitive {
enum EALIGN_TYPE {
	A = 0, P, L, R, I, S, TYPE_END
};

const glm::mat4 kRotateA = glm::mat4(1.0f);
const glm::mat4 kRotateP = glm::rotate(glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
const glm::mat4 kRotateS = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
const glm::mat4 kRotateI = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
const glm::mat4 kRotateL = glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
const glm::mat4 kRotateR = glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

class UIPRIMITIVE_EXPORT CW3ViewAlignTextItem : public QObject, public QGraphicsItem {
	Q_OBJECT
		Q_INTERFACES(QGraphicsItem)
public:
	CW3ViewAlignTextItem();
	~CW3ViewAlignTextItem();

signals:
	void sigRotateMatrix(const glm::mat4& mat);

public:
	void setVisible(bool visible);
	void setPosItem(int viewWidth);
	bool IsHovered();

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual QRectF boundingRect() const { return QRectF(); }

private slots:
	void slotPressedText();

private:
	std::vector<CW3TextItem*> directions_;
};
