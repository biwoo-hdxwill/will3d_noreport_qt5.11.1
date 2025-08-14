#include "clipping_tool.h"

#include <QButtonGroup>
#include <QRadioButton>
#include <QBoxLayout>
#include <QCheckBox>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/UIModule/UIPrimitive/W3SpanSlider.h>

#include "tool_box.h"

ClippingTool::ClippingTool(const std::vector<QString>& mode_names,
						   const DirectionType& direction, bool using_toolbox,
						   QObject* parent) :
	BaseTool(parent), direction_(direction) {
	modes_.resize(mode_names.size());

	CreateUI();
	if (using_toolbox) {
		clip_tool_box_.reset(new ToolBox());
		clip_tool_box_->setCaptionName(lang::LanguagePack::txt_clipping(), Qt::AlignLeft);
		clip_tool_box_->addToolLayout(clipping_layout_.get());
		clip_tool_box_->setContentsMargins(CW3Theme::getInstance()->getToolVBarSizeInfo().marginBox);
	}

	ResetUI();
	SetToolTips();
	Connections();

	for (int id = 0; id < mode_names.size(); ++id)
		modes_[id]->setText(mode_names[id]);
}

ClippingTool::~ClippingTool() {
	CW3LayoutFunctions::RemoveWidgetsAll(clipping_layout_.get());
}

void ClippingTool::ResetUI() {
	modes_[0]->setChecked(true);

	enable_->setCheckState(Qt::CheckState::Unchecked);
	flip_->setCheckState(Qt::CheckState::Unchecked);

	enable_->setDisabled(false);
	SetEnableUI(false);

	slider_->setLowerValue(0);
	slider_->setUpperValue(50);
}

void ClippingTool::SetEnable(bool enable) {
	if (!enable)
		SetEnableUI(enable);

	enable_->setEnabled(enable);
}

void ClippingTool::SetEnableUI(bool is_enable) {
	for (int id = 0; id < modes_.size(); ++id) {
		modes_[id]->setEnabled(is_enable);
	}
	flip_->setEnabled(is_enable);
	slider_->setEnabled(is_enable);
}


int ClippingTool::GetClipPlaneID() {
	for (int id = 0; id < modes_.size(); ++id) {
		if (modes_[id]->isChecked())
			return id;
	}
	return -1;
}

bool ClippingTool::IsClipEnable() {
	return enable_->isChecked();
}

bool ClippingTool::IsFlip() {
	return flip_->isChecked();
}

int ClippingTool::GetLowerValue() const {
	return slider_->lowerValue();
}

int ClippingTool::GetUpperValue() const {
	return slider_->upperValue();
}

void ClippingTool::SetLowerValue(const int & value) {
	slider_->setLowerValue(value);
}

void ClippingTool::SetUpperValue(const int & value) {
	slider_->setUpperValue(value);
}

void ClippingTool::SetClipParams(const bool & is_enable, const bool & is_flip,
								 const int & clip_id, const int & lower, const int & upper) {
	blockSignals(true);
	modes_[clip_id]->setChecked(true);
	slider_->setSpan(lower, upper);

	flip_->setChecked(is_flip);
	enable_->setChecked(is_enable);
	blockSignals(false);

	emit sigPlaneChanged(clip_id);
}

QWidget * ClippingTool::GetWidget() {
	return clip_tool_box_.get();
}

QLayout * ClippingTool::GetLayoutOnly() {
	return clipping_layout_.get();
}

void ClippingTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	int spacingL = theme->getToolVBarSizeInfo().spacingL;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;
	float spacingS = theme->getToolVBarSizeInfo().spacingS;
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;

	clipping_layout_.reset(new QVBoxLayout());
	clipping_layout_->setSpacing(spacingM);

	// create enable, flip buttons
	QString general_text_stylesheet = theme->toolGeneralTextStyleSheet();
	QHBoxLayout* active_buttons_layout = new QHBoxLayout();
	active_buttons_layout->setContentsMargins(contentsMargin);
	active_buttons_layout->setSpacing(0);
	enable_.reset(new QCheckBox());
	enable_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	enable_->setObjectName("GeneralText");
	enable_->setStyleSheet(general_text_stylesheet);
	enable_->setText(lang::LanguagePack::txt_enable());
	enable_->setEnabled(true);
	active_buttons_layout->addWidget(enable_.get());

	flip_.reset(new QCheckBox());
	flip_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	flip_->setObjectName("GeneralText");
	flip_->setStyleSheet(general_text_stylesheet);
	flip_->setText(lang::LanguagePack::txt_flip());
	flip_->setEnabled(true);
	active_buttons_layout->addWidget(flip_.get());

	active_buttons_layout->setStretch(0, 1);
	active_buttons_layout->setStretch(1, 1);

	clipping_layout_->addLayout(active_buttons_layout);

	// create mode buttons
	mode_group_.reset(new QButtonGroup());
	for (int id = 0; id < modes_.size(); ++id) {
		modes_[id].reset(new QRadioButton());
		modes_[id]->setObjectName("GeneralText");
		modes_[id]->setStyleSheet(general_text_stylesheet);
		mode_group_->addButton(modes_[id].get());
	}

	if (direction_ == DirectionType::GRID) {
		QGridLayout* mode_layout = new QGridLayout();
		mode_layout->setContentsMargins(contentsMargin);
		mode_layout->setHorizontalSpacing(0);
		mode_layout->setVerticalSpacing(spacingM);

		for (int id = 0; id < modes_.size(); ++id) {
			mode_layout->addWidget(modes_[id].get(), static_cast<int>(id / 2), id % 2);
		}
		mode_layout->setColumnStretch(0, 1);
		mode_layout->setColumnStretch(1, 1);
		clipping_layout_->addLayout(mode_layout);
	} else {
		QVBoxLayout* mode_layout = new QVBoxLayout();
		mode_layout->setContentsMargins(QMargins(spacingS, 0, spacingS, spacingS));
		mode_layout->setSpacing(spacingM);

		for (int id = 0; id < modes_.size(); ++id) {
			mode_layout->addWidget(modes_[id].get());
		}
		clipping_layout_->addLayout(mode_layout);
	}

	// create slider
	slider_.reset(new CW3SpanSlider());
	slider_->setRange(0, 100);
	slider_->setContentsMargins(ui_tools::kMarginZero);
	slider_->setOrientation(Qt::Horizontal);
	slider_->setObjectName(lang::LanguagePack::txt_clipping());

	QVBoxLayout* slider_layout = new QVBoxLayout();
	slider_layout->setSpacing(0);
	slider_layout->setContentsMargins(contentsMargin);
	slider_layout->addWidget(slider_.get());

	clipping_layout_->addLayout(slider_layout);
}

void ClippingTool::Connections() {
#if 0
	connect(flip_.get(), SIGNAL(pressed()), this, SLOT(slotFlip()));
#else
	connect(flip_.get(), &QCheckBox::stateChanged, this, &ClippingTool::slotFlip);
#endif

	connect(slider_.get(), SIGNAL(sigSpanChanged(int, int)), this, SIGNAL(sigRangeMove(int, int)));
	connect(slider_.get(), SIGNAL(sigStopChangeValue()), this, SIGNAL(sigRangeSet()));

	for (int id = 0; id < modes_.size(); ++id)
		connect(modes_[id].get(), &QRadioButton::released, this, &ClippingTool::slotPlaneChanged);

	connect(enable_.get(), &QCheckBox::stateChanged, this, &ClippingTool::slotClipEnable);
}

void ClippingTool::SetToolTips() {

}

void ClippingTool::slotPlaneChanged() {
	QObject* sender = QObject::sender();
	for (int id = 0; id < modes_.size(); ++id) {
		if (sender == modes_[id].get()) {
			emit sigPlaneChanged(id);

			int v1 = slider_->lowerValue();
			int v2 = slider_->upperValue();
			emit sigRangeMove(v1, v2);
			break;
		}
	}
}

void ClippingTool::slotClipEnable(int state) {
	bool is_enable;
	if (state == Qt::CheckState::Checked) {
		is_enable = true;
	} else if (state == Qt::CheckState::Unchecked) {
		is_enable = false;
	}
	SetEnableUI(is_enable);
	
	emit sigEnable(state);
}

void ClippingTool::slotFlip(int state) {
	int v1 = slider_->lowerValue();
	int v2 = slider_->upperValue();
	if (abs(50 - v1) < abs(50 - v2))
		v2 = 100 - v2;
	else
		v1 = 100 - v1;

	slider_->setSpan(v1, v2);
	// setSpan 에서 sigMPRClipRangeMove() 가 emit 되지만
	// high quality render 하기 위해 sigRangeSet() emit 한다. 
#if 0
	emit sigRangeSet();
#else
	emit sigFlip(state);
#endif
}
