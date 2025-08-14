#include "W3SpanSlider.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QTimer>

namespace {
	const int kSliderWheelStep = 84;
	const int kChangeTimeoutInterval = 200;
}
CW3SpanSliderPrivate::CW3SpanSliderPrivate()
	:lower(0),
	upper(0),
	lowerPos(0),
	upperPos(0),
	offset(0),
	position(0),
	lastPressed(CW3SpanSlider::NoHandle),
	mainControl(CW3SpanSlider::LowerHandle),
	lowerPressed(QStyle::SC_None),
	upperPressed(QStyle::SC_None),
	movement(CW3SpanSlider::FreeMovement),
	firstMovement(false),
	blockTracking(false) {}

void CW3SpanSliderPrivate::initStyleOption(QStyleOptionSlider* option, CW3SpanSlider::SpanHandle handle) const {
	const CW3SpanSlider* p = q_ptr;
	p->initStyleOption(option);
	option->sliderPosition = (handle == CW3SpanSlider::LowerHandle ? lowerPos : upperPos);
	option->sliderValue = (handle == CW3SpanSlider::LowerHandle ? lower : upper);
}

int CW3SpanSliderPrivate::pixelPosToRangeValue(int pos) const {
	QStyleOptionSlider opt;
	initStyleOption(&opt);

	int sliderMin = 0;
	int sliderMax = 0;
	int sliderLength = 0;
	const QSlider* p = q_ptr;
	const QRect gr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
	const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
	if (p->orientation() == Qt::Horizontal) {
		sliderLength = sr.width();
		sliderMin = gr.x();
		sliderMax = gr.right() - sliderLength + 1;
	} else {
		sliderLength = sr.height();
		sliderMin = gr.y();
		sliderMax = gr.bottom() - sliderLength + 1;
	}
	return QStyle::sliderValueFromPosition(p->minimum(), p->maximum(), pos - sliderMin,
										   sliderMax - sliderMin, opt.upsideDown);
}

void CW3SpanSliderPrivate::handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, CW3SpanSlider::SpanHandle handle) {
	QStyleOptionSlider opt;
	initStyleOption(&opt, handle);
	CW3SpanSlider* p = q_ptr;
	const QStyle::SubControl oldControl = control;
	control = p->style()->hitTestComplexControl(QStyle::CC_Slider, &opt, pos, p);
	const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
	if (control == QStyle::SC_SliderHandle) {
		position = value;
		offset = pick(pos - sr.topLeft());
		lastPressed = handle;
		p->setSliderDown(true);
		emit p->sigSliderPressed(handle);
	}
	if (control != oldControl)
		p->update(sr);
}

void CW3SpanSliderPrivate::setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const {
	QColor spanColor = q_ptr->palette().color(QPalette::AlternateBase);

	QLinearGradient gradient(x1, y1, x2, y2);
	gradient.setColorAt(0, spanColor);
	gradient.setColorAt(0.5, spanColor);
	gradient.setColorAt(1, spanColor.dark(120));
	painter->setBrush(gradient);

	painter->setPen(QPen(spanColor.dark(150), 0));
}

void CW3SpanSliderPrivate::drawSpan(QStylePainter* painter, const QRect& rect) const {
	QStyleOptionSlider opt;
	initStyleOption(&opt);
	const QSlider* p = q_ptr;

	// area
	QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
	QRect drawRect = rect;

	if (opt.orientation == Qt::Horizontal)
		drawRect.adjust(0, -1, 0, 1);
	else
		drawRect.adjust(-1, 0, 1, 0);

	// pen & brush
	painter->setPen(QPen(p->palette().color(QPalette::Dark).light(110), 0));
	if (opt.orientation == Qt::Horizontal)
		setupPainter(painter, opt.orientation, groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
	else
		setupPainter(painter, opt.orientation, groove.left(), groove.center().y(), groove.right(), groove.center().y());

	// draw groove
	painter->drawRect(drawRect.intersected(groove));
}

void CW3SpanSliderPrivate::drawHandle(QStylePainter* painter, CW3SpanSlider::SpanHandle handle) const {
	QStyleOptionSlider opt;
	initStyleOption(&opt, handle);
	opt.subControls = QStyle::SC_SliderHandle;
	QStyle::SubControl pressed = (handle == CW3SpanSlider::LowerHandle ? lowerPressed : upperPressed);
	if (pressed == QStyle::SC_SliderHandle) {
		opt.activeSubControls = pressed;
		opt.state |= QStyle::State_Sunken;
	}
	painter->drawComplexControl(QStyle::CC_Slider, opt);
}

void CW3SpanSliderPrivate::triggerAction(QAbstractSlider::SliderAction action, bool main) {
	int value = 0;
	bool no = false;
	bool up = false;
	const int min = q_ptr->minimum();
	const int max = q_ptr->maximum();
	const CW3SpanSlider::SpanHandle altControl = (mainControl == CW3SpanSlider::LowerHandle ? CW3SpanSlider::UpperHandle : CW3SpanSlider::LowerHandle);

	blockTracking = true;

	switch (action) {
	case QAbstractSlider::SliderSingleStepAdd:
		if ((main && mainControl == CW3SpanSlider::UpperHandle) || (!main && altControl == CW3SpanSlider::UpperHandle)) {
			value = qBound(min, upper + q_ptr->singleStep(), max);
			up = true;
			break;
		}
		value = qBound(min, lower + q_ptr->singleStep(), max);
		break;
	case QAbstractSlider::SliderSingleStepSub:
		if ((main && mainControl == CW3SpanSlider::UpperHandle) || (!main && altControl == CW3SpanSlider::UpperHandle)) {
			value = qBound(min, upper - q_ptr->singleStep(), max);
			up = true;
			break;
		}
		value = qBound(min, lower - q_ptr->singleStep(), max);
		break;
	case QAbstractSlider::SliderToMinimum:
		value = min;
		if ((main && mainControl == CW3SpanSlider::UpperHandle) || (!main && altControl == CW3SpanSlider::UpperHandle))
			up = true;
		break;
	case QAbstractSlider::SliderToMaximum:
		value = max;
		if ((main && mainControl == CW3SpanSlider::UpperHandle) || (!main && altControl == CW3SpanSlider::UpperHandle))
			up = true;
		break;
	case QAbstractSlider::SliderMove:
		if ((main && mainControl == CW3SpanSlider::UpperHandle) || (!main && altControl == CW3SpanSlider::UpperHandle))
			up = true;
	case QAbstractSlider::SliderNoAction:
		no = true;
		break;
	default:
		qWarning("CW3SpanSliderPrivate::triggerAction: Unknown action");
		break;
	}

	if (!no && !up) {
		if (movement == CW3SpanSlider::NoCrossing)
			value = qMin(value, upper);
		else if (movement == CW3SpanSlider::NoOverlapping)
			value = qMin(value, upper - 1);

		if (movement == CW3SpanSlider::FreeMovement && value > upper) {
			swapControls();
			q_ptr->setUpperPosition(value);
		} else {
			q_ptr->setLowerPosition(value);
		}
	} else if (!no) {
		if (movement == CW3SpanSlider::NoCrossing)
			value = qMax(value, lower);
		else if (movement == CW3SpanSlider::NoOverlapping)
			value = qMax(value, lower + 1);

		if (movement == CW3SpanSlider::FreeMovement && value < lower) {
			swapControls();
			q_ptr->setLowerPosition(value);
		} else {
			q_ptr->setUpperPosition(value);
		}
	}

	blockTracking = false;
	q_ptr->setLowerValue(lowerPos);
	q_ptr->setUpperValue(upperPos);
}

void CW3SpanSliderPrivate::swapControls() {
	qSwap(lower, upper);
	qSwap(lowerPressed, upperPressed);
	lastPressed = (lastPressed == CW3SpanSlider::LowerHandle ? CW3SpanSlider::UpperHandle : CW3SpanSlider::LowerHandle);
	mainControl = (mainControl == CW3SpanSlider::LowerHandle ? CW3SpanSlider::UpperHandle : CW3SpanSlider::LowerHandle);
}

void CW3SpanSliderPrivate::updateRange(int min, int max) {
	Q_UNUSED(min);
	Q_UNUSED(max);
	// setSpan() takes care of keeping span in range
	q_ptr->setSpan(lower, upper);
}

void CW3SpanSliderPrivate::movePressedHandle() {
	switch (lastPressed) {
	case CW3SpanSlider::LowerHandle:
		if (lowerPos != lower) {
			bool main = (mainControl == CW3SpanSlider::LowerHandle);
			triggerAction(QAbstractSlider::SliderMove, main);
		}
		break;
	case CW3SpanSlider::UpperHandle:
		if (upperPos != upper) {
			bool main = (mainControl == CW3SpanSlider::UpperHandle);
			triggerAction(QAbstractSlider::SliderMove, main);
		}
		break;
	default:
		break;
	}
}

/*!
Constructs a new CW3SpanSlider with \a parent.
*/
CW3SpanSlider::CW3SpanSlider(QWidget* parent) : QSlider(parent), d_ptr(new CW3SpanSliderPrivate()) {
	d_ptr->q_ptr = this;
	this->setStyleSheet("QSlider::sub-page:horizontal {"
						"	border-image: url();"
						"	background: url(); "
						"}");
	connect(this, SIGNAL(rangeChanged(int, int)), d_ptr, SLOT(updateRange(int, int)));
	connect(this, SIGNAL(sliderReleased()), d_ptr, SLOT(movePressedHandle()));

	change_timer_ = new QTimer;
	connect(change_timer_, SIGNAL(timeout()), this, SLOT(slotChangeTimeOut()));
}

/*!
Constructs a new CW3SpanSlider with \a orientation and \a parent.
*/
CW3SpanSlider::CW3SpanSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent), d_ptr(new CW3SpanSliderPrivate()) {
	d_ptr->q_ptr = this;
	this->setStyleSheet("QSlider::sub-page:horizontal {"
						"	border-image: url();"
						"	background: url(); "
						"}");
	connect(this, SIGNAL(rangeChanged(int, int)), d_ptr, SLOT(updateRange(int, int)));
	connect(this, SIGNAL(sliderReleased()), d_ptr, SLOT(movePressedHandle()));


	change_timer_ = new QTimer;
	connect(change_timer_, SIGNAL(timeout()), this, SLOT(slotChangeTimeOut()));
}

CW3SpanSlider::~CW3SpanSlider() {
	if(change_timer_)
		delete change_timer_;
}

/*!
\property CW3SpanSlider::handleMovementMode
\brief the handle movement mode
*/
CW3SpanSlider::HandleMovementMode CW3SpanSlider::handleMovementMode() const {
	return d_ptr->movement;
}

void CW3SpanSlider::setHandleMovementMode(CW3SpanSlider::HandleMovementMode mode) {
	d_ptr->movement = mode;
}

/*!
\property CW3SpanSlider::lowerValue
\brief the lower value of the span
*/
int CW3SpanSlider::lowerValue() const {
	return qMin(d_ptr->lower, d_ptr->upper);
}

void CW3SpanSlider::setLowerValue(int lower) {
	setSpan(lower, upperValue());
}

/*!
\property CW3SpanSlider::upperValue
\brief the upper value of the span
*/
int CW3SpanSlider::upperValue() const {
	return qMax(d_ptr->lower, d_ptr->upper);
}

void CW3SpanSlider::setUpperValue(int upper) {
	setSpan(lowerValue(), upper);
}

/*!
Sets the span from \a lower to \a upper.
*/
void CW3SpanSlider::setSpan(int lower, int upper) {
	const int low = qBound(minimum(), qMin(lower, upper), maximum());
	const int upp = qBound(minimum(), qMax(lower, upper), maximum());
	if (low != d_ptr->lower || upp != d_ptr->upper) {
		if (low != d_ptr->lower) {
			d_ptr->lower = low;
			d_ptr->lowerPos = low;
			emit sigLowerValueChanged(low);
		}
		if (upp != d_ptr->upper) {
			d_ptr->upper = upp;
			d_ptr->upperPos = upp;
			emit sigUpperValueChanged(upp);
		}
		emit sigSpanChanged(d_ptr->lower, d_ptr->upper);
		update();
	}
}

/*!
\property CW3SpanSlider::lowerPosition
\brief the lower position of the span
*/
int CW3SpanSlider::lowerPosition() const {
	return d_ptr->lowerPos;
}

void CW3SpanSlider::setLowerPosition(int lower) {
	if (d_ptr->lowerPos != lower) {
		d_ptr->lowerPos = lower;
		if (!hasTracking())
			update();
		if (isSliderDown())
			emit sigLowerPositionChanged(lower);
		if (hasTracking() && !d_ptr->blockTracking) {
			bool main = (d_ptr->mainControl == CW3SpanSlider::LowerHandle);
			d_ptr->triggerAction(SliderMove, main);
		}
	}
}

/*!
\property CW3SpanSlider::upperPosition
\brief the upper position of the span
*/
int CW3SpanSlider::upperPosition() const {
	return d_ptr->upperPos;
}

void CW3SpanSlider::setUpperPosition(int upper) {
	if (d_ptr->upperPos != upper) {
		d_ptr->upperPos = upper;
		if (!hasTracking())
			update();
		if (isSliderDown())
			emit sigUpperPositionChanged(upper);
		if (hasTracking() && !d_ptr->blockTracking) {
			bool main = (d_ptr->mainControl == CW3SpanSlider::UpperHandle);
			d_ptr->triggerAction(SliderMove, main);
		}
	}
}

void CW3SpanSlider::stopChangeValue() {
	emit sigStopChangeValue();
}

void CW3SpanSlider::keyPressEvent(QKeyEvent* event) {
	bool main = true;
	SliderAction action = SliderNoAction;
	switch (event->key()) {
	case Qt::Key_Left:
		main = (orientation() == Qt::Horizontal);
		action = !invertedAppearance() ? SliderSingleStepSub : SliderSingleStepAdd;
		break;
	case Qt::Key_Right:
		main = (orientation() == Qt::Horizontal);
		action = !invertedAppearance() ? SliderSingleStepAdd : SliderSingleStepSub;
		break;
	case Qt::Key_Up:
		main = (orientation() == Qt::Vertical);
		action = invertedControls() ? SliderSingleStepSub : SliderSingleStepAdd;
		break;
	case Qt::Key_Down:
		main = (orientation() == Qt::Vertical);
		action = invertedControls() ? SliderSingleStepAdd : SliderSingleStepSub;
		break;
	case Qt::Key_Home:
		main = (d_ptr->mainControl == CW3SpanSlider::LowerHandle);
		action = SliderToMinimum;
		break;
	case Qt::Key_End:
		main = (d_ptr->mainControl == CW3SpanSlider::UpperHandle);
		action = SliderToMaximum;
		break;
	default:
		event->ignore();
		break;
	}

	if (action) {
		d_ptr->triggerAction(action, main);
		change_timer_->start(kChangeTimeoutInterval);
	}

	QSlider::keyPressEvent(event);
}

void CW3SpanSlider::mousePressEvent(QMouseEvent* event) {
	if (minimum() == maximum() || (event->buttons() ^ event->button())) {
		event->ignore();
		return;
	}

	d_ptr->handleMousePress(event->pos(), d_ptr->upperPressed, d_ptr->upper, CW3SpanSlider::UpperHandle);
	if (d_ptr->upperPressed != QStyle::SC_SliderHandle)
		d_ptr->handleMousePress(event->pos(), d_ptr->lowerPressed, d_ptr->lower, CW3SpanSlider::LowerHandle);

	d_ptr->firstMovement = true;
	event->accept();
}

void CW3SpanSlider::mouseMoveEvent(QMouseEvent* event) {
	if (d_ptr->lowerPressed != QStyle::SC_SliderHandle && d_ptr->upperPressed != QStyle::SC_SliderHandle) {
		event->ignore();
		return;
	}

	QStyleOptionSlider opt;
	d_ptr->initStyleOption(&opt);
	const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
	int newPosition = d_ptr->pixelPosToRangeValue(d_ptr->pick(event->pos()) - d_ptr->offset);
	if (m >= 0) {
		const QRect r = rect().adjusted(-m, -m, m, m);
		if (!r.contains(event->pos())) {
			newPosition = d_ptr->position;
		}
	}

	// pick the preferred handle on the first movement
	if (d_ptr->firstMovement) {
		if (d_ptr->lower == d_ptr->upper) {
			if (newPosition < lowerValue()) {
				d_ptr->swapControls();
				d_ptr->firstMovement = false;
			}
		} else {
			d_ptr->firstMovement = false;
		}
	}

	if (d_ptr->lowerPressed == QStyle::SC_SliderHandle) {
		if (d_ptr->movement == NoCrossing)
			newPosition = qMin(newPosition, upperValue());
		else if (d_ptr->movement == NoOverlapping)
			newPosition = qMin(newPosition, upperValue() - 1);

		if (d_ptr->movement == FreeMovement && newPosition > d_ptr->upper) {
			d_ptr->swapControls();
			setUpperPosition(newPosition);
		} else {
			setLowerPosition(newPosition);
		}
	} else if (d_ptr->upperPressed == QStyle::SC_SliderHandle) {
		if (d_ptr->movement == NoCrossing)
			newPosition = qMax(newPosition, lowerValue());
		else if (d_ptr->movement == NoOverlapping)
			newPosition = qMax(newPosition, lowerValue() + 1);

		if (d_ptr->movement == FreeMovement && newPosition < d_ptr->lower) {
			d_ptr->swapControls();
			setLowerPosition(newPosition);
		} else {
			setUpperPosition(newPosition);
		}
	}
	event->accept();
}

void CW3SpanSlider::mouseReleaseEvent(QMouseEvent* event) {
	QSlider::mouseReleaseEvent(event);

	if (d_ptr->upperPressed == QStyle::SC_SliderHandle)
		emit sigSliderReleased(CW3SpanSlider::UpperHandle);
	else
		emit sigSliderReleased(CW3SpanSlider::LowerHandle);

	emit sigStopChangeValue();

	setSliderDown(false);
	d_ptr->lowerPressed = QStyle::SC_None;
	d_ptr->upperPressed = QStyle::SC_None;

	update();
}

void CW3SpanSlider::leaveEvent(QEvent * event) {
	QSlider::leaveEvent(event);
}

void CW3SpanSlider::enterEvent(QEvent * event) {
	QSlider::enterEvent(event);
	this->setFocus();
}

void CW3SpanSlider::wheelEvent(QWheelEvent * event) {

	QStyleOptionSlider opt;
	d_ptr->initStyleOption(&opt);
	const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
	int newPosition = d_ptr->pixelPosToRangeValue(d_ptr->pick(event->pos()) - d_ptr->offset);
	if (m >= 0) {
		const QRect r = rect().adjusted(-m, -m, m, m);
		if (!r.contains(event->pos())) {
			newPosition = d_ptr->position;
		}
	}

	int delta = event->delta() / kSliderWheelStep;
	if (abs(d_ptr->upperPos - newPosition) < abs(d_ptr->lowerPos - newPosition)) {		

		newPosition = d_ptr->upperPos + delta;
		newPosition = qBound(minimum(), newPosition, maximum());
		setUpperPosition(newPosition);
	} else {

		newPosition = d_ptr->lowerPos + delta;
		newPosition = qBound(minimum(), newPosition, maximum());
		setLowerPosition(newPosition);
	}
	change_timer_->start(kChangeTimeoutInterval);
	event->ignore();
}

void CW3SpanSlider::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event);
	QStylePainter painter(this);

	// groove & ticks
	QStyleOptionSlider opt;
	d_ptr->initStyleOption(&opt);
	opt.sliderValue = 0;
	opt.sliderPosition = 0;
	opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;
	painter.drawComplexControl(QStyle::CC_Slider, opt);

	// handle rects
	opt.sliderPosition = d_ptr->lowerPos;
	const QRect lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	const int lrv = d_ptr->pick(lr.center());
	opt.sliderPosition = d_ptr->upperPos;
	const QRect ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	const int urv = d_ptr->pick(ur.center());

	// span
	const int minv = qMin(lrv, urv);
	const int maxv = qMax(lrv, urv);
	const QPoint c = QRect(lr.center(), ur.center()).center();
	QRect spanRect;
	if (orientation() == Qt::Horizontal)
		spanRect = QRect(QPoint(minv, c.y() - 2), QPoint(maxv, c.y() + 1));
	else
		spanRect = QRect(QPoint(c.x() - 2, minv), QPoint(c.x() + 1, maxv));
	d_ptr->drawSpan(&painter, spanRect);

	// handles
	switch (d_ptr->lastPressed) {
	case CW3SpanSlider::LowerHandle:
		d_ptr->drawHandle(&painter, CW3SpanSlider::UpperHandle);
		d_ptr->drawHandle(&painter, CW3SpanSlider::LowerHandle);
		break;
	case CW3SpanSlider::UpperHandle:
	default:
		d_ptr->drawHandle(&painter, CW3SpanSlider::LowerHandle);
		d_ptr->drawHandle(&painter, CW3SpanSlider::UpperHandle);
		break;
	}
}
void CW3SpanSlider::slotChangeTimeOut() {
	change_timer_->stop();
	emit sigStopChangeValue();
}
