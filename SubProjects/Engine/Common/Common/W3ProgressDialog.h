#pragma once

/*=========================================================================

File:			class CW3ProgressDialog
Language:		C++11
Library:		Qt 5.4.0
Author:			TaeHoon Yoo
First Date:		2016-11-03
Modify Date:	2016-11-03
Version:		1.0

=========================================================================*/
#include <QDialog>
#include "common_global.h"

class QTimer;

class COMMON_EXPORT CW3ProgressDialog : public QDialog {
	Q_OBJECT

public:
	enum Format {
		PERCENT,
		WAITTING
	};

public:
	CW3ProgressDialog(Format format = PERCENT, QWidget *parent = 0);
	~CW3ProgressDialog();

public:
	static void setInstance(QWidget* parent = 0) {
		if (m_pInstance == nullptr) {
			m_pInstance = new CW3ProgressDialog(PERCENT, parent);
		}
	}
	static CW3ProgressDialog* getInstance(Format format) {
		if (m_pInstance == nullptr) {
			m_pInstance = new CW3ProgressDialog();
			atexit(destroy);
		}
		m_pInstance->setFormat(format);
		return m_pInstance;
	}

	static const int PositionLeft = 180;
	static const int PositionTop = 90;
	static const int PositionRight = 0;
	static const int PositionBottom = -90;

	/**
	 * @brief Return position (in degrees) of minimum value.
	 * \sa setNullPosition
	 */
	float nullPosition() const { return m_nullPosition; }
	/**
	 * @brief Defines position of minimum value.
	 * @param position position on the circle (in degrees) of minimum value
	 * \sa nullPosition
	 */
	void setNullPosition(float position);

	/**
	 * @brief Returns current value shown on the widget.
	 * \sa setValue()
	 */
	float value() const { return m_value; }
	/**
	 * @brief Returns minimum of the allowed value range.
	 * \sa setMinimum, setRange
	 */
	float minimum() const { return m_min; }
	/**
	 * @brief Returns maximum of the allowed value range.
	* \sa setMaximum, setRange
	 */
	float maximum() const { return m_max; }

	void setFormat(Format format) { m_format = format; }

	void run();

	void runEnd();

	public slots:
	/**
	 * @brief Defines minimum und maximum of the allowed value range.
	 * If the current value does not fit into the range, it will be automatically adjusted.
	 * @param min minimum of the allowed value range
	 * @param max maximum of the allowed value range
	 */
	void setRange(float min, float max);
	/**
	 * @brief Defines minimum of the allowed value range.
	 * If the current value does not fit into the range, it will be automatically adjusted.
	 * @param min minimum of the allowed value range
	 * \sa setRange
	 */
	void setMinimum(float min);
	/**
	 * @brief Defines maximum of the allowed value range.
	 * If the current value does not fit into the range, it will be automatically adjusted.
	 * @param max maximum of the allowed value range
	 * \sa setRange
	 */
	void setMaximum(float max);
	/**
	 * @brief Sets a value which will be shown on the widget.
	 * @param val must be between minimum() and maximum()
	 */
	void setValue(float val);
	/**
	 * @brief Integer version of the previous slot.
	 * @param val must be between minimum() and maximum()
	 */
	void setValue(int val);

	int exec();
	void hide(bool stop = true);
	void show();

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void calculateInnerRect(float outerRadius, QRectF& innerRect, float innerRadius);
	virtual void drawBackground(QPainter& p, const QRectF& baseRect);
	virtual void drawShadowBackground(QPainter& p, const QRectF& baseRect);
	virtual void drawContourBackground(QPainter& p, const QRectF& innerRect);
	virtual void drawDataBackground(QPainter& p, const QRectF& innerRect);
	virtual void drawData(QPainter& p, const QRectF& innerRect, float value, float arcLength, float dataWidth);
	virtual void drawCenterBackground(QPainter& p, const QRectF& innerRect);
	virtual void drawText(QPainter& p, const QRectF& innerRect, float innerRadius, float value);

	virtual QSize minimumSizeHint() const { return QSize(150, 150); }

	virtual bool hasHeightForWidth() const { return true; }
	virtual int heightForWidth(int w) const { return w; }

	inline void rotate2Dpoint(QPointF& point, const QPointF& origin, float radian);

	float m_min, m_max;
	float m_value;
	float m_waitOffset;

	float m_nullPosition;
	QGradientStops m_gradientData;

	Format m_format;
	QTimer* m_pWaitDelayTimer;

	QString m_strLoading[4];
	int m_strLoadingStep;

private:
	static void destroy() {
		if (m_pInstance != nullptr)
			delete m_pInstance;
	}

private:
	static CW3ProgressDialog* m_pInstance;

	private slots:
	void slotWaitDelay();
};

