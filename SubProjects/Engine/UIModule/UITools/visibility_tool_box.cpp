#include "visibility_tool_box.h"

#include <QToolButton>
#include <QBoxLayout>
#include <QCheckBox>
#include <qlabel.h>
#include <QLineEdit>
#include <QSlider>
#include <qvalidator.h>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/nerve_resource.h>
#include <Engine/Resource/Resource/implant_resource.h>
#include <Engine/Resource/Resource/face_photo_resource.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#include "tool_box.h"

VisibilityToolBox::VisibilityToolBox(bool nerve_on, bool implant_on,
	bool second_on, bool airway_on,
	bool face_on, QObject* parent) :
	BaseTool(parent)
{
	active_option_[VisibleID::NERVE] = nerve_on;
//20250123 LIN
//#ifndef WILL3D_VIEWER
	active_option_[VisibleID::IMPLANT] = implant_on;
//#endif
#ifndef WILL3D_LIGHT
	active_option_[VisibleID::SECONDVOLUME] = second_on;
	active_option_[VisibleID::AIRWAY] = airway_on;
	active_option_[VisibleID::FACEPHOTO] = face_on;
#endif

	CreateUI();
	ResetUI();
	SetToolTips();
	Connections();
}

VisibilityToolBox::~VisibilityToolBox()
{
	tool_box_->ClearTools();
}

void VisibilityToolBox::ResetUI()
{
	for (int id = 0; id < VisibleID::VISIBLE_END; ++id)
	{
		if (active_option_[id])
		{
			options_[id]->setEnabled(false);
			options_[id]->setCheckState(Qt::CheckState::Unchecked);
		}
	}
	//20250123 LIN
//#ifndef WILL3D_VIEWER
	if (active_option_[VisibleID::IMPLANT])
	{
		blockSignals(true);
		adjust_implant_button_->setChecked(false);
		adjust_implant_button_->setEnabled(false);
		blockSignals(false);
	}
//#endif

#ifndef WILL3D_LIGHT
	if (active_option_[VisibleID::AIRWAY])
	{
		airway_size_->setEnabled(false);
		airway_size_->setText("0 " + QString::fromLocal8Bit("mm3"));
	}

	if (active_option_[VisibleID::FACEPHOTO])
	{
		lbl_face_->setEnabled(false);
		face_transparency_->setMaximum(100);
		face_transparency_->setMinimum(0);
		face_transparency_->setValue(100);
		face_transparency_->setDisabled(true);
	}
#endif
}

void VisibilityToolBox::SetAdjustImplantButtonVisible(const bool visible)
{
	//20250123 LIN
//#ifndef WILL3D_VIEWER
	if (!adjust_implant_button_)
	{
		return;
	}

	adjust_implant_button_->setVisible(visible);
//#endif
}

void VisibilityToolBox::SetAirwaySize(float size)
{
#ifndef WILL3D_LIGHT
	if (!active_option_[VisibleID::AIRWAY])
		return;

	QString suffix = QString::fromLocal8Bit("mm3");
	airway_size_->setText(QString::number(size) + " " + suffix);
#endif
}

void VisibilityToolBox::EnableFaceUI(bool enable)
{
#ifndef WILL3D_LIGHT
	SyncVisibilityEnable(VisibleID::FACEPHOTO, enable);
	options_[VisibleID::FACEPHOTO]->setChecked(enable);
#endif
}

void VisibilityToolBox::EnableAirwayUI(bool enable)
{

#ifndef WILL3D_LIGHT
	SyncVisibilityEnable(VisibleID::AIRWAY, enable);
	if (!enable)
		options_[VisibleID::AIRWAY]->setChecked(enable);
#endif
}

void VisibilityToolBox::SyncVisibilityResources()
{
	for (int id = 0; id < VisibleID::VISIBLE_END; ++id)
	{
		if (!active_option_[id])
			continue;

#ifndef WILL3D_LIGHT
		if (id == VisibleID::AIRWAY)
			continue;
#endif

		bool is_enable = IsResourceEnable(static_cast<VisibleID>(id));
		bool curr_enable = options_[id]->isEnabled();

		if (is_enable != curr_enable)
		{
			options_[id]->setEnabled(is_enable);

//20250123 LIN implant ?곕씪??蹂댁뿬???섏꽌 二쇱꽍泥섎━??
//#ifndef WILL3D_VIEWER
			if (id == VisibleID::IMPLANT)
			{
				adjust_implant_button_->setEnabled(is_enable);
			}
//#endif
#ifndef WILL3D_LIGHT
			else if (id == VisibleID::FACEPHOTO)
			{
				lbl_face_->setEnabled(is_enable);
				face_transparency_->setEnabled(is_enable);
			}
#endif

			options_[id]->setChecked(is_enable);
		}
	}
}

void VisibilityToolBox::SyncVisibilityEnable(const VisibleID& visible_id, const bool& enable)
{
	if (!active_option_[visible_id])
		return;

	options_[visible_id]->setEnabled(enable);
//20250123 LIN
//#ifndef WILL3D_VIEWER
	if (visible_id == VisibleID::IMPLANT)
	{
		adjust_implant_button_->setEnabled(enable);
	}
//#endif
#ifndef WILL3D_LIGHT
	else if (visible_id == VisibleID::AIRWAY)
	{
		if (!enable)
		{
			airway_size_->setText("0 " + QString::fromLocal8Bit("mm3"));
		}
	}
	else if (visible_id == VisibleID::FACEPHOTO)
	{
		lbl_face_->setEnabled(enable);
		face_transparency_->setEnabled(enable);
	}
#endif
}

void VisibilityToolBox::SyncVisibilityChecked(const VisibleID& visible_id, const bool& checked)
{
	if (!active_option_[visible_id])
	{
		return;
	}

	options_[visible_id]->setChecked(checked);
}

void VisibilityToolBox::SetVisibleResource(const VisibleID & visible_id, const bool & visible)
{
	if (!active_option_[visible_id])
		return;

	options_[visible_id]->setChecked(visible);
}

bool VisibilityToolBox::IsEnable(const VisibleID & visible_id) const
{
	if (!active_option_[visible_id])
		return false;

	return options_[visible_id]->isEnabled();
}

bool VisibilityToolBox::IsChecked(const VisibleID & visible_id) const
{
	if (!active_option_[visible_id])
		return false;

	return options_[visible_id]->isChecked();
}

QWidget * VisibilityToolBox::GetWidget()
{
	return tool_box_.get();
}

void VisibilityToolBox::CreateUI()
{
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;
	int spacingS = theme->getToolVBarSizeInfo().spacingS;

	QVBoxLayout* visibility_layout = new QVBoxLayout();
	visibility_layout->setSpacing(6);
	visibility_layout->setContentsMargins(contentsMargin);

	for (int id = 0; id < VisibleID::VISIBLE_END; ++id)
	{
		if (active_option_[id])
		{
			options_[id].reset(new QCheckBox());
			options_[id]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			options_[id]->setEnabled(false);
		}
	}
//20250123 LIN visibility tool ?숈씪?섍쾶 留뚮벉 
//#ifndef WILL3D_VIEWER 
	if (active_option_[VisibleID::NERVE] && active_option_[VisibleID::IMPLANT])
	{
		options_[VisibleID::NERVE]->setText(lang::LanguagePack::txt_nerve());
		options_[VisibleID::IMPLANT]->setText(lang::LanguagePack::txt_implant());

		adjust_implant_button_ = new QToolButton();
		adjust_implant_button_->setText("A");
		adjust_implant_button_->setCheckable(true);
		adjust_implant_button_->setFixedSize(15, 15);
		adjust_implant_button_->setVisible(false);
		adjust_implant_button_->setEnabled(false);

		options_[VisibleID::NERVE]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		options_[VisibleID::IMPLANT]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		QHBoxLayout* implant_layout = new QHBoxLayout();
		implant_layout->setContentsMargins(ui_tools::kMarginZero);
		implant_layout->setSpacing(3);
		implant_layout->addWidget(options_[VisibleID::IMPLANT].get(), 0, Qt::AlignLeft | Qt::AlignVCenter);
		implant_layout->addWidget(adjust_implant_button_, 0, Qt::AlignLeft | Qt::AlignVCenter);

		QHBoxLayout* nerve_implant_layout = new QHBoxLayout();
		nerve_implant_layout->setContentsMargins(ui_tools::kMarginZero);
		nerve_implant_layout->setSpacing(spacingM);
		nerve_implant_layout->addWidget(options_[VisibleID::NERVE].get(), 1, Qt::AlignLeft | Qt::AlignVCenter);
		nerve_implant_layout->addLayout(implant_layout, 1);
		visibility_layout->addLayout(nerve_implant_layout);
	}
//#else
//	if (active_option_[VisibleID::NERVE])
//	{
//		options_[VisibleID::NERVE]->setText(lang::LanguagePack::txt_nerve());
//
//		adjust_implant_button_ = new QToolButton();
//		adjust_implant_button_->setText("A");
//		adjust_implant_button_->setCheckable(true);
//		adjust_implant_button_->setFixedSize(15, 15);
//		adjust_implant_button_->setVisible(false);
//		adjust_implant_button_->setEnabled(false);
//
//		options_[VisibleID::NERVE]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//
//		QHBoxLayout* nerve_layout = new QHBoxLayout();
//		nerve_layout->setContentsMargins(ui_tools::kMarginZero);
//		nerve_layout->setSpacing(spacingM);
//		nerve_layout->addWidget(options_[VisibleID::NERVE].get(), 1, Qt::AlignLeft | Qt::AlignVCenter);
//		visibility_layout->addLayout(nerve_layout);
//	}
//#endif

#ifndef WILL3D_LIGHT
	if (active_option_[VisibleID::SECONDVOLUME])
	{
		options_[VisibleID::SECONDVOLUME]->setText(lang::LanguagePack::txt_2nd_vol());

		visibility_layout->addWidget(options_[VisibleID::SECONDVOLUME].get());
	}

	if (active_option_[VisibleID::AIRWAY])
	{
		options_[VisibleID::AIRWAY]->setText(lang::LanguagePack::txt_airway());

		QHBoxLayout *airway_layout = new QHBoxLayout();
		airway_layout->setContentsMargins(ui_tools::kMarginZero);
		airway_layout->setSpacing(spacingM);
		airway_layout->addWidget(options_[VisibleID::AIRWAY].get());

		airway_size_.reset(new QLineEdit());
		airway_size_->setStyleSheet(theme->toolVisibilityStyleSheet());
		airway_size_->setReadOnly(true);
		airway_size_->setAlignment(Qt::AlignRight);
		airway_size_->setValidator(new QDoubleValidator(0, std::numeric_limits<double>::max(),
			2, airway_size_.get()));
		airway_size_->setFixedHeight(16);
		airway_layout->addWidget(airway_size_.get());
		visibility_layout->addLayout(airway_layout);
	}

	if (active_option_[VisibleID::FACEPHOTO])
	{
		options_[VisibleID::FACEPHOTO]->setText(lang::LanguagePack::txt_face_photo());

		lbl_face_.reset(new QLabel);
		lbl_face_->setText(lang::LanguagePack::txt_face_transparency());
		lbl_face_->setAlignment(Qt::AlignLeft);

		face_transparency_.reset(new QSlider());
		face_transparency_->setStyleSheet(theme->appQSliderStyleSheet());
		face_transparency_->setContentsMargins(ui_tools::kMarginZero);
		face_transparency_->setOrientation(Qt::Horizontal);
		face_transparency_->setMaximum(100);
		face_transparency_->setMinimum(0);

		QVBoxLayout* slider_layout = new QVBoxLayout();
		slider_layout->setSpacing(spacingS);
		slider_layout->setContentsMargins(ui_tools::kMarginZero);
		slider_layout->addWidget(lbl_face_.get());
		slider_layout->addWidget(face_transparency_.get());

		visibility_layout->addWidget(options_[VisibleID::FACEPHOTO].get());
		visibility_layout->addLayout(slider_layout);
	}
#endif

	tool_box_.reset(new ToolBox());
	tool_box_->setCaptionName(lang::LanguagePack::txt_visibility(), Qt::AlignLeft);
	tool_box_->addToolLayout(visibility_layout);
	tool_box_->setContentsMargins(theme->getToolVBarSizeInfo().marginBox);
}

void VisibilityToolBox::Connections()
{
	if (active_option_[VisibleID::NERVE])
	{
		connect(options_[VisibleID::NERVE].get(), SIGNAL(stateChanged(int)),
			this, SLOT(slotStateChangeNerve(int)));
	}

	if (active_option_[VisibleID::IMPLANT])
	{
		connect(options_[VisibleID::IMPLANT].get(), SIGNAL(stateChanged(int)),
			this, SLOT(slotStateChangeImplant(int)));
	}
	if (adjust_implant_button_)
	{
		connect(adjust_implant_button_, &QToolButton::toggled, this, &VisibilityToolBox::sigAdjustImplantButtonToggled);
	}

#ifndef WILL3D_LIGHT
	if (active_option_[VisibleID::AIRWAY])
	{
		connect(options_[VisibleID::AIRWAY].get(), SIGNAL(stateChanged(int)),
			this, SLOT(slotStateChangeAirway(int)));
	}

	if (active_option_[VisibleID::FACEPHOTO])
	{
		connect(options_[VisibleID::FACEPHOTO].get(), SIGNAL(stateChanged(int)),
			this, SLOT(slotStateChangeFace(int)));
		connect(face_transparency_.get(), SIGNAL(valueChanged(int)), this,
			SIGNAL(sigChangeFaceTransparency(int)));
	}

	if (active_option_[VisibleID::SECONDVOLUME])
	{
		connect(options_[VisibleID::SECONDVOLUME].get(), SIGNAL(stateChanged(int)),
			this, SLOT(slotStateChangeSecondVol(int)));
	}
#endif
}

void VisibilityToolBox::SetToolTips()
{
}

bool VisibilityToolBox::IsResourceEnable(const VisibleID & id)
{
	switch (id)
	{
	case VisibleID::NERVE:
		return ResourceContainer::GetInstance()->GetNerveResource().IsSetNervePoints();
	case VisibleID::IMPLANT:
		return ResourceContainer::GetInstance()->GetImplantResource().IsSetImplant();
#ifndef WILL3D_LIGHT
	case VisibleID::SECONDVOLUME:
		return &ResourceContainer::GetInstance()->GetSecondVolume() != nullptr;
	case VisibleID::AIRWAY:
		// smseo : resource container 占쏙옙 airway占쏙옙 占쌔븝옙占쏙옙占?占십억옙, Tab占쏙옙占쏙옙 sync 占싹게뀐옙 占쌔듸옙.
		return false;
	case VisibleID::FACEPHOTO:
		return ResourceContainer::GetInstance()->GetFacePhotoResource().IsSetFace();
#endif
	}
	return false;
}

void VisibilityToolBox::slotStateChangeFace(int state)
{
#ifndef WILL3D_LIGHT
	switch (state)
	{
	case Qt::CheckState::Checked:
		lbl_face_->setEnabled(true);
		face_transparency_->setEnabled(true);
		break;
	case Qt::CheckState::Unchecked:
		lbl_face_->setEnabled(false);
		face_transparency_->setEnabled(false);
		break;
	}

	emit sigVisible(VisibleID::FACEPHOTO, state);
#endif
}

void VisibilityToolBox::slotStateChangeSecondVol(int state)
{
#ifndef WILL3D_LIGHT
	emit sigVisible(VisibleID::SECONDVOLUME, state);
#endif
}

void VisibilityToolBox::slotStateChangeNerve(int state)
{
	emit sigVisible(VisibleID::NERVE, state);
}

void VisibilityToolBox::slotStateChangeImplant(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		adjust_implant_button_->setEnabled(true);
		break;
	case Qt::CheckState::Unchecked:
		adjust_implant_button_->setEnabled(false);
		break;
	}

	emit sigVisible(VisibleID::IMPLANT, state);
}

void VisibilityToolBox::slotStateChangeAirway(int state)
{
#ifndef WILL3D_LIGHT
	switch (state)
	{
	case Qt::CheckState::Checked:
		airway_size_->setEnabled(true);
		break;
	case Qt::CheckState::Unchecked:
		airway_size_->setEnabled(false);
		break;
	}

	emit sigVisible(VisibleID::AIRWAY, state);
#endif
}
