#pragma once

/*=========================================================================

File:			class GenerateFaceDlg
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-07-21
Last modify:	2016-07-21

=========================================================================*/
#include "../../Common/Common/W3Dialog.h"
#include "uiframe_global.h"

class QSlider;
class QDoubleSpinBox;

class UIFRAME_EXPORT GenerateFaceDlg : public CW3Dialog {
	Q_OBJECT

public:
	GenerateFaceDlg(QWidget* parent = 0);
	~GenerateFaceDlg();

public:
	void SetThreshold(double threshold);
	inline double threshold() const { return threshold_; }

signals:
	void sigChangeValue(double value);
	void sigThresholdEditingFinished();

private slots:
	void slotChangeSpinThreshold(double);
	void slotChangeSliderThreshold(int);

private:
	QDoubleSpinBox* threshold_spin_ = nullptr;
	QSlider* threshold_slider_ = nullptr;
	double threshold_ = 0.0;
	QTimer* timer_ = nullptr;
};
