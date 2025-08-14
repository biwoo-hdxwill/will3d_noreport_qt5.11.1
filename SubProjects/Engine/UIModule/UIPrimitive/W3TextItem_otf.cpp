#include "W3TextItem_otf.h"
#include <qstring.h>
#include <qfont.h>
#include <QApplication>

CW3TextItem_otf::CW3TextItem_otf(const QPointF pt, const float val)
{
	QString str("[" + QString().setNum(val, 'f', 0) + "]");
	this->setPlainText(str);

	QFont font = QApplication::font();
	font.setWeight(QFont::Bold);
	this->setFont(font);
	this->setDefaultTextColor(Qt::white);
	this->setFlag(QGraphicsTextItem::ItemIgnoresTransformations, true);
	this->setPos(pt.x(), pt.y());
}

CW3TextItem_otf::~CW3TextItem_otf(void)
{
}
