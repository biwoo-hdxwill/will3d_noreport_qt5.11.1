#include "W3ViewAlignTextItem.h"

#include <QApplication>

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/define_ui.h"
#include "W3TextItem.h"

using namespace UIPrimitive;

CW3ViewAlignTextItem::CW3ViewAlignTextItem() {
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);

	for (int i = A; i < TYPE_END; i++) {
		CW3TextItem* text = new CW3TextItem(this);
		text->setFont(font);
		connect(text, SIGNAL(sigPressed()), this, SLOT(slotPressedText()));

		directions_.push_back(text);
	}
	directions_[A]->setPlainText("A");
	directions_[P]->setPlainText("P");
	directions_[L]->setPlainText("L");
	directions_[R]->setPlainText("R");
	directions_[I]->setPlainText("I");
	directions_[S]->setPlainText("S");
}

CW3ViewAlignTextItem::~CW3ViewAlignTextItem() {
	for (auto elem : directions_)
		SAFE_DELETE_OBJECT(elem);
}
void CW3ViewAlignTextItem::setVisible(bool visible) {
	for (auto& item : directions_) {
		item->setVisible(visible);
	}

	QGraphicsItem::setVisible(visible);
}
void CW3ViewAlignTextItem::setPosItem(int viewWidth) {
	float horizontalSpacing = directions_[A]->sceneBoundingRect().width();

	for (int i = 0; i < directions_.size(); i++) {
		directions_[i]->setPos(mapToScene(viewWidth - common::ui_define::kViewMarginWidth - (horizontalSpacing * (6 - i)),
										  common::ui_define::kViewFilterOffsetY));
	}
}
bool CW3ViewAlignTextItem::IsHovered() {
	for (const auto& elem : directions_) {
		if (elem->isHovered())
			return true;
	}

	return false;
}
void CW3ViewAlignTextItem::slotPressedText() {
	QObject* pSender = QObject::sender();
	if (pSender == directions_[S])
		emit sigRotateMatrix(kRotateS);
	else if (pSender == directions_[I])
		emit sigRotateMatrix(kRotateI);
	else if (pSender == directions_[L])
		emit sigRotateMatrix(kRotateL);
	else if (pSender == directions_[R])
		emit sigRotateMatrix(kRotateR);
	else if (pSender == directions_[A])
		emit sigRotateMatrix(kRotateA);
	else if (pSender == directions_[P])
		emit sigRotateMatrix(kRotateP);
}
