#include "W3ProgressDialog.h"

#include "qmath.h"
#include <QtGui/QPainter>
#include <QTimer>
#include <QApplication>

#include "W3Memory.h"
#include "language_pack.h"

CW3ProgressDialog* CW3ProgressDialog::m_pInstance = nullptr;

CW3ProgressDialog::CW3ProgressDialog(Format format, QWidget *parent) :
	m_format(format),
	QDialog(parent, Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint)
{
	this->setAttribute(Qt::WA_TranslucentBackground);
	m_min = 0.0f;
	m_max = 100.0f;
	m_value = 0.0f;
	m_nullPosition = PositionTop;
	                                    
	m_waitOffset = 1.0f;

	this->setFixedSize(150, 150);

	m_strLoading[0] = QString(lang::LanguagePack::txt_loading());
	m_strLoading[1] = QString(lang::LanguagePack::txt_loading() + ".");
	m_strLoading[2] = QString(lang::LanguagePack::txt_loading() + "..");
	m_strLoading[3] = QString(lang::LanguagePack::txt_loading() + "...");

	m_strLoadingStep = 0;

	m_pWaitDelayTimer = nullptr;
}

CW3ProgressDialog::~CW3ProgressDialog()
{
	if (m_pWaitDelayTimer)
		SAFE_DELETE_OBJECT(m_pWaitDelayTimer);
}

void CW3ProgressDialog::setRange(float min, float max)
{
	m_min = min;
	m_max = max;

	if (m_max < m_min)
		qSwap(m_max, m_min);

	if (m_value < m_min)
		m_value = m_min;
	else if (m_value > m_max)
		m_value = m_max;

	m_waitOffset = (m_max - m_min) / 100.0f;
}

void CW3ProgressDialog::run()
{
	if (m_pWaitDelayTimer == nullptr)
	{
		m_pWaitDelayTimer = new QTimer(this);
		connect(m_pWaitDelayTimer, SIGNAL(timeout()), this, SLOT(slotWaitDelay()));
	}

	if (m_pWaitDelayTimer->isActive())
	{
		return;
	}

	setValue(m_min);

	m_pWaitDelayTimer->start(10);
}

void CW3ProgressDialog::runEnd()
{
	if (m_pWaitDelayTimer)
		SAFE_DELETE_OBJECT(m_pWaitDelayTimer);

	QApplication::processEvents();
}

void CW3ProgressDialog::setMinimum(float min)
{
	setRange(min, m_max);
}

void CW3ProgressDialog::setMaximum(float max)
{
	setRange(m_min, max);
}

void CW3ProgressDialog::setValue(float val)
{
	if (m_value != val)
	{
		if (val < m_min)
			m_value = m_min;
		else if (val > m_max)
			m_value = m_max;
		else
			m_value = val;
	}
}

void CW3ProgressDialog::setValue(int val)
{
	setValue((float)val);
}

int CW3ProgressDialog::exec()
{
	//if (m_format == WAITTING)
	this->run();

	raise();
	return QDialog::exec();
}

void CW3ProgressDialog::hide(bool stop)
{
	//if (m_format == WAITTING)
	if (stop)
	{
		this->runEnd();
	}

	QDialog::hide();
}

void CW3ProgressDialog::show()
{
	run();

	QDialog::show();
}

void CW3ProgressDialog::setNullPosition(float position)
{
	if (position != m_nullPosition)
	{
		m_nullPosition = position;
	}
}

void CW3ProgressDialog::slotWaitDelay()
{
	if (m_format == WAITTING)
	{
		float val = m_value;
		val += m_waitOffset;

		if (val > m_max)
			val -= m_max;

		setValue(val);
	}
	else if (m_format == PERCENT)
	{
		++m_strLoadingStep;
		if (m_strLoadingStep > 399)
			m_strLoadingStep = 0;
	}

	update();
}

void CW3ProgressDialog::paintEvent(QPaintEvent* /*event*/)
{
	float outerRadius = qMin(width(), height());
	QRectF baseRect(1, 1, outerRadius - 2, outerRadius - 2);

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	float contourRadius = outerRadius*0.9f;
	float dataRadius = outerRadius*0.8f;
	float centerRadius = outerRadius*0.6f;

	// shadow circle
	drawShadowBackground(p, baseRect);

	// contour circle
	QRectF contourRect;
	calculateInnerRect(outerRadius, contourRect, contourRadius);
	drawContourBackground(p, contourRect);

	// data circle
	QRectF dataRect;
	calculateInnerRect(outerRadius, dataRect, dataRadius);
	drawDataBackground(p, dataRect);

	float arcStep = 360.0f / (m_max - m_min) * m_value;
	float dataWidth = (dataRadius - centerRadius)*0.5f;
	drawData(p, dataRect, m_value, arcStep, dataWidth);

	// center circle
	QRectF centerRect;
	calculateInnerRect(outerRadius, centerRect, centerRadius);

	drawCenterBackground(p, centerRect);

	// text
	if (m_format == PERCENT)
		drawText(p, centerRect, centerRadius, m_value);
}

void CW3ProgressDialog::calculateInnerRect(float outerRadius, QRectF &innerRect, float innerRadius)
{
	float delta = (outerRadius - innerRadius) / 2;
	innerRect = QRectF(delta, delta, innerRadius, innerRadius);
}

void CW3ProgressDialog::drawBackground(QPainter &p, const QRectF &baseRect)
{
	p.fillRect(baseRect, QColor(0, 0, 0, 0));
}

void CW3ProgressDialog::drawShadowBackground(QPainter &p, const QRectF &baseRect)
{
	QRadialGradient rg;
	rg.setCenter(baseRect.center());
	rg.setFocalPoint(baseRect.center());
	rg.setRadius(baseRect.width()*0.5f);

	rg.setColorAt(1.0f, QColor(255, 255, 255, 0));
	rg.setColorAt(0.97f, QColor(134, 134, 134, 255));
	rg.setColorAt(0.0f, QColor(134, 134, 134, 255));

	p.setPen(QColor(0, 0, 0, 0));
	p.setBrush(rg);
	p.drawEllipse(baseRect);
}

void CW3ProgressDialog::drawContourBackground(QPainter & p, const QRectF & innerRect)
{
	p.setPen(QColor(0, 0, 0, 0));
	p.setBrush(QColor(66, 66, 66, 255));
	p.drawEllipse(innerRect);
}

void CW3ProgressDialog::drawDataBackground(QPainter & p, const QRectF & innerRect)
{
	p.setPen(QColor(0, 0, 0, 0));

	QLinearGradient lg;
	lg.setColorAt(0.0f, QColor(63, 63, 63, 255));
	lg.setColorAt(1.0f, QColor(37, 37, 37, 255));
	lg.setStart(0, 0);
	lg.setFinalStop(0, innerRect.height());
	p.setBrush(lg);
	p.drawEllipse(innerRect);

	QRadialGradient rg;
	rg.setCenter(innerRect.center());
	rg.setFocalPoint(innerRect.center());
	rg.setRadius(innerRect.width()*0.5f);

	rg.setColorAt(1.0f, QColor(37, 37, 37, 140));
	rg.setColorAt(0.97f, QColor(37, 37, 37, 30));
	rg.setColorAt(0.96f, QColor(37, 37, 37, 0));

	p.setBrush(rg);
	p.drawEllipse(innerRect);
}

void CW3ProgressDialog::drawData(QPainter &p, const QRectF &innerRect, float value, float arcLength, float dataWidth)
{
	// nothing to draw
	if (value == m_min)
		return;

	// for Pie and Donut styles
	QPainterPath dataPath;
	dataPath.setFillRule(Qt::WindingFill);

	QConicalGradient cg;
	QRadialGradient rg;
	p.setPen(QColor(0, 0, 0, 0));


	// effect rounded pie
	QPointF headPoint = QPointF(innerRect.center().x(), innerRect.y() + dataWidth*0.5);
	rotate2Dpoint(headPoint, innerRect.center(), arcLength / 180.0f * M_PI);
	QRectF headRect = QRectF(headPoint.x() - dataWidth*0.5f, headPoint.y() - dataWidth*0.5f, dataWidth, dataWidth);
	dataPath.addEllipse(headRect);

	if (m_format == PERCENT)
	{
		// pie segment outer
		dataPath.moveTo(innerRect.center());
		dataPath.arcTo(innerRect, m_nullPosition, -arcLength);
		dataPath.lineTo(innerRect.center());

		cg.setCenter(innerRect.center());
		cg.setAngle(90.0f);
		cg.setColorAt(1.0f - arcLength / 360.0f, QColor(109, 148, 228, 255));
		cg.setColorAt(1.0f, QColor(130, 171, 255, 255));

		rg.setCenter(innerRect.center());
		rg.setFocalPoint(innerRect.center());
		rg.setRadius(innerRect.width()*0.5f);

		rg.setColorAt(1.0f, QColor(100, 100, 100, 120));
		rg.setColorAt(0.96f, QColor(160, 160, 160, 60));
		rg.setColorAt(0.95f, QColor(160, 160, 160, 0));

		p.setBrush(cg);
		p.drawPath(dataPath);

		p.setBrush(rg);
		p.drawPath(dataPath);
	}
	else
	{
		dataPath.moveTo(innerRect.center());
		dataPath.arcTo(innerRect, -arcLength - 90, -180);
		dataPath.lineTo(innerRect.center());

		cg.setCenter(innerRect.center());
		cg.setAngle(-arcLength - 90.0f);

		cg.setColorAt(0.5f, QColor(109, 148, 228, 255));
		cg.setColorAt(1.0f, QColor(130, 171, 255, 0));
		rg.setCenter(innerRect.center());
		rg.setFocalPoint(innerRect.center());
		rg.setRadius(innerRect.width()*0.5f);

		rg.setColorAt(1.0f, QColor(100, 100, 100, 120));
		rg.setColorAt(0.96f, QColor(160, 160, 160, 60));
		rg.setColorAt(0.95f, QColor(160, 160, 160, 0));

		p.setBrush(cg);
		p.drawPath(dataPath);
	}
}

void CW3ProgressDialog::drawCenterBackground(QPainter &p, const QRectF &innerRect)
{
	p.setPen(QColor(0, 0, 0, 0));

	QLinearGradient lg;
	lg.setColorAt(0.0f, QColor(120, 120, 120, 255));
	lg.setColorAt(1.0f, QColor(63, 63, 63, 255));
	lg.setStart(0, 0);
	lg.setFinalStop(0, innerRect.height());
	p.setBrush(lg);
	p.drawEllipse(innerRect);

	QRadialGradient rg;
	rg.setCenter(innerRect.center());

	QPointF center = innerRect.center();
	QPointF focal;

	QRectF shadowRect = innerRect.adjusted(2, 2, 0, 0);

	focal = QPointF(innerRect.x(), center.y());

	rotate2Dpoint(focal, innerRect.center(), M_PI*0.25f);
	focal += QPointF(0.01f, 0.01f);

	rg.setFocalPoint(focal);
	rg.setRadius(innerRect.width()*0.5f);

	rg.setColorAt(1.0f, QColor(33, 33, 33, 170));
	rg.setColorAt(0.98f, QColor(63, 63, 63, 80));
	rg.setColorAt(0.97f, QColor(63, 63, 63, 0));

	p.setBrush(rg);
	p.drawEllipse(shadowRect);

	QRectF highlightRect = innerRect.adjusted(0, 0, -2, -2);

	focal = QPointF(innerRect.right(), center.y());
	rotate2Dpoint(focal, innerRect.center(), M_PI*0.25f);
	focal -= QPointF(0.01f, 0.01f);

	rg.setFocalPoint(focal);
	rg.setRadius(innerRect.width()*0.5f);

	rg.setColorAt(1.0f, QColor(190, 190, 190, 200));
	rg.setColorAt(0.99f, QColor(120, 120, 120, 120));
	rg.setColorAt(0.98f, QColor(120, 120, 120, 0));

	p.setBrush(rg);
	p.drawEllipse(highlightRect);
}

void CW3ProgressDialog::drawText(QPainter &p, const QRectF &innerRect, float innerRadius, float value)
{
	QFont f = QApplication::font();
	f.setPixelSize(innerRadius * 0.32f);
	f.setWeight(QFont::Light);
	f.setLetterSpacing(QFont::AbsoluteSpacing, -innerRadius*0.02f);
	p.setFont(f);

	QRectF textRect(innerRect);
	p.setPen(Qt::white);

	QString txtValue("%v %");

	int procent = (int)((value - m_min) / (m_max - m_min) * 100.0f);
	txtValue.replace("%v", QString::number(procent));

	p.translate(0, -f.pixelSize()*0.15f);
	p.drawText(textRect, Qt::AlignCenter, txtValue);
	p.translate(0, f.pixelSize()*0.8f);

	f.setPixelSize(innerRadius * 0.115f);
	f.setLetterSpacing(QFont::AbsoluteSpacing, 1);
	p.setFont(f);

	int lIdx = (int)(m_strLoadingStep / 100);
	p.drawText(textRect, Qt::AlignCenter, m_strLoading[lIdx]);
}

inline void CW3ProgressDialog::rotate2Dpoint(QPointF & point, const QPointF& origin, float radian)
{
	point = QPointF((point.x() - origin.x())*cos(radian) - (point.y() - origin.y())*sin(radian) + origin.x(),
		(point.x() - origin.x())*sin(radian) + (point.y() - origin.y())*cos(radian) + origin.y());
}
