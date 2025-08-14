#include "generate_face_dialog.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QTimer>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/language_pack.h"

GenerateFaceDlg::GenerateFaceDlg(QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_set_thd_for_face_model(), parent) {
	timer_ = new QTimer(this);
	timer_->setInterval(300);
	timer_->setSingleShot(true);
	connect(timer_, SIGNAL(timeout()), this, SIGNAL(sigThresholdEditingFinished()));

	CW3Theme* theme = CW3Theme::getInstance();

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(15, 10, 15, 5);
	layout->setSpacing(10);

	QHBoxLayout* threshold_layout = new QHBoxLayout();
	threshold_layout->setSpacing(5);

	QLabel* text = new QLabel();
	text->setText(lang::LanguagePack::txt_thd_value());

	threshold_spin_ = new QDoubleSpinBox();
	threshold_spin_->setRange(0.0, 3000.0);
	threshold_spin_->setValue(900.5);
	threshold_spin_->setSingleStep(0.5);

	threshold_layout->addWidget(text);
	threshold_layout->addWidget(threshold_spin_);

	threshold_slider_ = new QSlider();
	threshold_slider_->setStyleSheet(theme->appQSliderStyleSheet());
	threshold_slider_->setContentsMargins(0, 0, 0, 0);
	threshold_slider_->setOrientation(Qt::Horizontal);
	threshold_slider_->setRange(0.0, 3000.0);
	threshold_slider_->setValue(900.5);

	QHBoxLayout* slider_layout = new QHBoxLayout();
	slider_layout->setContentsMargins(0, 0, 0, 0);
	slider_layout->setSpacing(0);
	slider_layout->setAlignment(Qt::AlignCenter);

	slider_layout->addWidget(threshold_slider_);

	layout->addLayout(threshold_layout);
	layout->addLayout(slider_layout);

	QHBoxLayout* command_layout = new QHBoxLayout();
	command_layout->setContentsMargins(15, 5, 15, 10);
	command_layout->setSpacing(5);
	command_layout->setAlignment(Qt::AlignCenter);
	QToolButton* ok_button = new QToolButton();
	ok_button->setText(lang::LanguagePack::txt_ok());
	QToolButton* cancel_button = new QToolButton();
	cancel_button->setText(lang::LanguagePack::txt_cancel());
	command_layout->addWidget(ok_button);
	command_layout->addWidget(cancel_button);

	m_contentLayout->addLayout(layout);
	m_contentLayout->addLayout(command_layout);

	connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

	connect(threshold_spin_, SIGNAL(valueChanged(double)), this, SLOT(slotChangeSpinThreshold(double)));
	connect(threshold_slider_, SIGNAL(valueChanged(int)), this, SLOT(slotChangeSliderThreshold(int)));
#if 1
	connect(threshold_spin_, SIGNAL(editingFinished()), this, SIGNAL(sigThresholdEditingFinished()));
	connect(threshold_slider_, SIGNAL(sliderReleased()), this, SIGNAL(sigThresholdEditingFinished()));
#endif

	hide();
}

GenerateFaceDlg::~GenerateFaceDlg() {
	SAFE_DELETE_OBJECT(timer_);
}

void GenerateFaceDlg::SetThreshold(double threshold) {
	threshold_spin_->setValue(threshold);
	threshold_slider_->setValue(threshold);
	threshold_ = threshold;
}

void GenerateFaceDlg::slotChangeSpinThreshold(double value) {
	threshold_ = value;
	threshold_slider_->setValue(static_cast<int>(value));
	emit sigChangeValue(value);

	timer_->start();
}

void GenerateFaceDlg::slotChangeSliderThreshold(int value) {
	threshold_spin_->setValue(static_cast<double>(value));
}
