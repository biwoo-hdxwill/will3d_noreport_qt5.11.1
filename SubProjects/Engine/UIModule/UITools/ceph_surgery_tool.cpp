#include "ceph_surgery_tool.h"

#include <qboxlayout.h>
#include <QGridLayout>
#include <QSignalMapper>
#include <QLabel>
#include <QToolButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QSettings>
#include <QTextCodec>

#include <Engine/Common/Common/language_pack.h>
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/language_pack.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_ceph.h"
#endif

using namespace surgery;

namespace {
const glm::vec3 kAxisAxial(0.0f, 0.0f, 1.0f);
const glm::vec3 kAxisCoronal(0.0f, 1.0f, 0.0f);
const glm::vec3 kAxisSagittal(-1.0f, 0.0f, 0.0f);
} // end of namespace

CephSurgeryTool::CephSurgeryTool(QWidget* parent)
	:QFrame(parent) {
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	QString sectionKey = "CEPH/surgery_on_axis_visible";
	bool is_enable_move = settings.value(sectionKey).toBool();

	for (int i = 0; i < CUT_TYPE_END; i++)
		is_move_[i] = is_enable_move;

	this->setObjectName("SurgeryBar");

	QVBoxLayout * main_layout = new QVBoxLayout();
	main_layout->setAlignment(Qt::AlignTop);
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->setSpacing(0);

	QHBoxLayout* captionLayout = new QHBoxLayout();
	captionLayout->setSpacing(0);
	captionLayout->setContentsMargins(0, 0, 0, 0);

	QLabel* caption = new QLabel();
	caption->setContentsMargins(10, 0, 0, 0);
	caption->setMinimumHeight(30);
	caption->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	caption->setObjectName("Caption");
	caption->setText(lang::LanguagePack::txt_3d_surgery());

	m_btnClose = new QToolButton();
	m_btnClose->setObjectName("Close");

	captionLayout->addWidget(caption);
	captionLayout->addWidget(m_btnClose);

	QVBoxLayout* contentLayout = new QVBoxLayout();
	contentLayout->setSpacing(0);
	contentLayout->setContentsMargins(0, 0, 0, 0);
	contentLayout->setAlignment(Qt::AlignTop);

	this->setContentLayout(contentLayout);

	main_layout->addLayout(captionLayout);
	main_layout->addLayout(contentLayout);

	this->setStyleSheet(CW3Theme::getInstance()->cephSurgeryBarStyleSheet());
	this->setLayout(main_layout);

	this->connections();

	setVisible(false);
}

CephSurgeryTool::~CephSurgeryTool() {}
#ifndef WILL3D_VIEWER
void CephSurgeryTool::exportProject(ProjectIOCeph & out) {
	// m_lstChkBox, m_isAdjust, m_isMove
	// -> ceph view에서 저장되었으므로 저장할 필요 없음
	
	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id) {
		QString status_text_qstr = ui_[id].adjust->text();
		std::string status_text = status_text_qstr.toLocal8Bit().toStdString();
		out.SaveSurgeryBtnStatusText(id * 2, status_text);

		status_text_qstr = ui_[id].move->text();
		status_text = status_text_qstr.toLocal8Bit().toStdString();
		out.SaveSurgeryBtnStatusText(id * 2 + 1, status_text);
	}

	int param_cnt = CutTypeID::CUT_TYPE_END * (TranslateID::TRANS_END + RotateID::ROT_END);
	std::vector<float> params(param_cnt);
	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id) {
		params[id*6] = static_cast<float>(ui_[id].trans[TranslateID::RL]->value());
		params[id*6 + 1] = static_cast<float>(ui_[id].trans[TranslateID::AP]->value());
		params[id*6 + 2] = static_cast<float>(ui_[id].trans[TranslateID::SI]->value());
		params[id*6 + 3] = static_cast<float>(ui_[id].rotate[RotateID::SAG]->value());
		params[id*6 + 4] = static_cast<float>(ui_[id].rotate[RotateID::COR]->value());
		params[id*6 + 5] = static_cast<float>(ui_[id].rotate[RotateID::AXI]->value());
	}
	out.SaveSrugeryParams(params);

	std::vector<float> prev(param_cnt);
	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id) {
		prev[id * 6] = prev_params_[id].trans[TranslateID::RL];
		prev[id * 6 + 1] = prev_params_[id].trans[TranslateID::AP];
		prev[id * 6 + 2] = prev_params_[id].trans[TranslateID::SI];
		prev[id * 6 + 3] = prev_params_[id].rotate[RotateID::SAG];
		prev[id * 6 + 4] = prev_params_[id].rotate[RotateID::COR];
		prev[id * 6 + 5] = prev_params_[id].rotate[RotateID::AXI];
	}
	out.SaveSurgeryParamsPrev(prev);
}

void CephSurgeryTool::importProject(ProjectIOCeph & in) {
	// m_lstChkBox, m_isAdjust, m_isMove
	// -> ceph view에서 저장되었으므로 로드하여 값 세팅만 해준다.
	bool maxilla, mandible, chin;
	in.LoadIsSurgeryCut(maxilla, mandible, chin);
	ui_[CutTypeID::MAXILLA].enable->setChecked(maxilla);
	ui_[CutTypeID::MANDIBLE].enable->setChecked(mandible);
	ui_[CutTypeID::CHIN].enable->setChecked(chin);

	in.LoadIsSurgeryAdjust(is_adjust_[MAXILLA], is_adjust_[MANDIBLE],
						   is_adjust_[CHIN]);

	in.LoadIsSurgeryMove(is_move_[MAXILLA], is_move_[MANDIBLE],
						 is_move_[CHIN]);

	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id) {
		std::string status_text;
		in.LoadSurgeryBtnStatusText(id * 2, status_text);
		ui_[id].adjust->setText(status_text.c_str());

		in.LoadSurgeryBtnStatusText(id * 2 + 1, status_text);
		ui_[id].move->setText(status_text.c_str());
	}

	std::vector<float> params;
	in.LoadSurgeryParams(params);
	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id) {
		ui_[id].trans[TranslateID::RL]->setValue(params[id * 6]);
		ui_[id].trans[TranslateID::AP]->setValue(params[id * 6 + 1]);
		ui_[id].trans[TranslateID::SI]->setValue(params[id * 6 + 2]);
		ui_[id].rotate[RotateID::SAG]->setValue(params[id * 6 + 3]);
		ui_[id].rotate[RotateID::COR]->setValue(params[id * 6 + 4]);
		ui_[id].rotate[RotateID::AXI]->setValue(params[id * 6 + 5]);
	}

	std::vector<float> prev;
	in.LoadSurgeryParamsPrev(prev);
	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id) {
		prev_params_[id].trans[TranslateID::RL] = prev[id * 6];
		prev_params_[id].trans[TranslateID::AP] = prev[id * 6 + 1];
		prev_params_[id].trans[TranslateID::SI] = prev[id * 6 + 2];
		prev_params_[id].rotate[RotateID::SAG] = prev[id * 6 + 3];
		prev_params_[id].rotate[RotateID::COR] = prev[id * 6 + 4];
		prev_params_[id].rotate[RotateID::AXI] = prev[id * 6 + 5];
	}
}
#endif
void CephSurgeryTool::slotSurgeryParamsClear(const surgery::CutTypeID& cut_id) {
	ui_[cut_id].trans[TranslateID::RL]->setValue(0.0);
	ui_[cut_id].trans[TranslateID::AP]->setValue(0.0);
	ui_[cut_id].trans[TranslateID::SI]->setValue(0.0);
	ui_[cut_id].rotate[RotateID::SAG]->setValue(0.0);
	ui_[cut_id].rotate[RotateID::COR]->setValue(0.0);
	ui_[cut_id].rotate[RotateID::AXI]->setValue(0.0);
}

void CephSurgeryTool::slotSurgeryParamsDisable() {
	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id)
		ui_[id].enable->setChecked(false);
}

void CephSurgeryTool::slotRotateFromView(const surgery::CutTypeID& cut_id,
										 const surgery::RotateID& rot_id,
										 float rotate_value) {
	float degree = ui_[cut_id].rotate[rot_id]->value() + rotate_value;
	if (degree > 180.0f)
		degree -= 360.0f;
	else if (degree < -180.0f)
		degree += 360.0f;

	glm::mat4 m4 = glm::mat4(1.0f);
	m4 *= glm::rotate(static_cast<float>(glm::radians(ui_[cut_id].rotate[RotateID::AXI]->value())), kAxisAxial);
	m4 *= glm::rotate(static_cast<float>(glm::radians(ui_[cut_id].rotate[RotateID::COR]->value())), kAxisCoronal);
	m4 *= glm::rotate(static_cast<float>(glm::radians(ui_[cut_id].rotate[RotateID::SAG]->value())), kAxisSagittal);

	disconnect(ui_[cut_id].rotate[rot_id], SIGNAL(valueChanged(double)), this, SLOT(slotRotate(double)));
	ui_[cut_id].rotate[rot_id]->setValue(degree);
	prev_params_[cut_id].rotate[rot_id] = degree;
	connect(ui_[cut_id].rotate[rot_id], SIGNAL(valueChanged(double)), this, SLOT(slotRotate(double)));

	emit sigSurgeryCutRotate(cut_id, m4);
}

void CephSurgeryTool::slotRotate(double value) {
	int cut_id, rotate_id;
	QObject* sender = QObject::sender();
	bool found = false;
	for (cut_id = 0; cut_id < CutTypeID::CUT_TYPE_END; ++cut_id) {
		for (rotate_id = 0; rotate_id < RotateID::ROT_END; ++rotate_id) {
			if (ui_[cut_id].rotate[rotate_id] == sender) {
				found = true;
				break;
			}
		}
		if (found)
			break;
	}

	glm::mat4 m4 = glm::mat4(1.0f);
	m4 *= glm::rotate(static_cast<float>(glm::radians(ui_[cut_id].rotate[RotateID::AXI]->value())), kAxisAxial);
	m4 *= glm::rotate(static_cast<float>(glm::radians(ui_[cut_id].rotate[RotateID::COR]->value())), kAxisCoronal);
	m4 *= glm::rotate(static_cast<float>(glm::radians(ui_[cut_id].rotate[RotateID::SAG]->value())), kAxisSagittal);

	prev_params_[cut_id].rotate[rotate_id] = ui_[cut_id].rotate[rotate_id]->value();
	CutTypeID cut_type = static_cast<CutTypeID>(cut_id);
	emit sigSurgeryAxisRotate(cut_type, m4);
}

void CephSurgeryTool::slotTranslateFromView(const surgery::CutTypeID& cut_id,
											const glm::vec3& trans) {
	float value[TranslateID::TRANS_END] = {
		(float)ui_[cut_id].trans[TranslateID::RL]->value() + trans.x,
		(float)ui_[cut_id].trans[TranslateID::AP]->value() + trans.y,
		(float)ui_[cut_id].trans[TranslateID::SI]->value() + trans.z,
	};

	glm::mat4 m4 = glm::translate(
		glm::vec3(prev_params_[cut_id].trans[TranslateID::RL] - value[TranslateID::RL],
				  prev_params_[cut_id].trans[TranslateID::AP] - value[TranslateID::AP],
				  prev_params_[cut_id].trans[TranslateID::SI] - value[TranslateID::SI]));

	for (int trans_id = 0; trans_id < TranslateID::TRANS_END; ++trans_id){
		disconnect(ui_[cut_id].trans[trans_id], SIGNAL(valueChanged(double)), this, SLOT(slotTranslate(double)));
		ui_[cut_id].trans[trans_id]->setValue(value[trans_id]);
		prev_params_[cut_id].trans[trans_id] = value[trans_id];
		connect(ui_[cut_id].trans[trans_id], SIGNAL(valueChanged(double)), this, SLOT(slotTranslate(double)));
	}

	emit sigSurgeryCutTranslate(cut_id, m4);
}

void CephSurgeryTool::slotTranslate(double value) {
	QObject* sender = QObject::sender();
	bool found = false;
	int cut_id, trans_id;
	for (cut_id = 0; cut_id < CutTypeID::CUT_TYPE_END; ++cut_id) {
		for (trans_id = 0; trans_id < TranslateID::TRANS_END; ++trans_id) {
			if (ui_[cut_id].trans[trans_id] == sender) {
				found = true;
				break;
			}
		}
		if (found)
			break;
	}

	glm::mat4 m4 = glm::translate(glm::vec3(
		prev_params_[cut_id].trans[TranslateID::RL] - ui_[cut_id].trans[TranslateID::RL]->value(),
		prev_params_[cut_id].trans[TranslateID::AP] - ui_[cut_id].trans[TranslateID::AP]->value(),
		prev_params_[cut_id].trans[TranslateID::SI] - ui_[cut_id].trans[TranslateID::SI]->value()));

	prev_params_[cut_id].trans[trans_id] = ui_[cut_id].trans[trans_id]->value();
	CutTypeID cut_type = static_cast<CutTypeID>(cut_id);
	emit sigSurgeryAxisTranslate(cut_type, m4);
}

void CephSurgeryTool::slotChangeEnable() {
	is_enable_ = !is_enable_;
	emit sigSurgeryModeEnable(is_enable_);
}

void CephSurgeryTool::connections() {
	connect(m_btnClose, SIGNAL(clicked()), this, SLOT(slotChangeEnable()));
}

void CephSurgeryTool::setContentLayout(QVBoxLayout* layout) {
	this->initControlItems();

	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id) {
		QFrame* main_frame = new QFrame();
		main_frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		main_frame->setObjectName("Content");

		QVBoxLayout* main_layout = new QVBoxLayout();
		main_layout->setSpacing(15);
		main_layout->setContentsMargins(15, 10, 15, 10);
		main_frame->setLayout(main_layout);

		QHBoxLayout* top_layout = new QHBoxLayout();
		top_layout->setSpacing(5);
		top_layout->addWidget(ui_[id].enable);
		top_layout->addWidget(ui_[id].adjust);
		top_layout->addWidget(ui_[id].move);
		main_layout->addLayout(top_layout);

		QGridLayout* bottom_layout = new QGridLayout();
		bottom_layout->setVerticalSpacing(5);
		bottom_layout->setHorizontalSpacing(10);
		bottom_layout->addWidget(new QLabel(lang::LanguagePack::txt_move() + "(mm)"), 0, 0, 1, 2);
		bottom_layout->addItem(new QSpacerItem(25, 1), 0, 2, 1, 1);
		bottom_layout->addWidget(new QLabel(lang::LanguagePack::txt_rotate() + "(" + lang::LanguagePack::txt_degree_short() + ")"), 0, 3, 1, 2);
		bottom_layout->addWidget(new QLabel("R-L"), 1, 0);
		bottom_layout->addWidget(ui_[id].trans[TranslateID::RL], 1, 1);
		bottom_layout->addItem(new QSpacerItem(25, 1), 1, 2);
		bottom_layout->addWidget(new QLabel(lang::LanguagePack::txt_sagittal()), 1, 3);
		bottom_layout->addWidget(ui_[id].rotate[RotateID::SAG], 1, 4);
		bottom_layout->addWidget(new QLabel("A-P"), 2, 0);
		bottom_layout->addWidget(ui_[id].trans[TranslateID::AP], 2, 1);
		bottom_layout->addItem(new QSpacerItem(25, 1), 2, 2);
		bottom_layout->addWidget(new QLabel(lang::LanguagePack::txt_coronal()), 2, 3);
		bottom_layout->addWidget(ui_[id].rotate[RotateID::COR], 2, 4);
		bottom_layout->addWidget(new QLabel("S-I"), 3, 0);
		bottom_layout->addWidget(ui_[id].trans[TranslateID::SI], 3, 1);
		bottom_layout->addItem(new QSpacerItem(25, 1), 3, 2);
		bottom_layout->addWidget(new QLabel(lang::LanguagePack::txt_axial()), 3, 3);
		bottom_layout->addWidget(ui_[id].rotate[RotateID::AXI], 3, 4);
		bottom_layout->setColumnStretch(1, 1);
		bottom_layout->setColumnStretch(4, 1);
		main_layout->addLayout(bottom_layout);

		layout->addWidget(main_frame);
	}
}

void CephSurgeryTool::initControlItems() {
	CW3Theme* theme = CW3Theme::getInstance();

	for (int id = 0; id < CutTypeID::CUT_TYPE_END; ++id) {
		ui_[id].enable = new QCheckBox();
		ui_[id].enable->setMinimumWidth(80);
		connect(ui_[id].enable, SIGNAL(stateChanged(int)), this, SLOT(slotEnable(int)));
		
		ui_[id].adjust = new QToolButton();
		ui_[id].adjust->setStyleSheet(theme->appToolButtonStyleSheet());
		ui_[id].adjust->setEnabled(false);
		ui_[id].adjust->setText(lang::LanguagePack::txt_adjust_cut());
		connect(ui_[id].adjust, SIGNAL(clicked()), this, SLOT(slotAdjustOn()));

		ui_[id].move = new QToolButton();
		ui_[id].move->setStyleSheet(theme->appToolButtonStyleSheet());
		ui_[id].move->setEnabled(false);
		connect(ui_[id].move, SIGNAL(clicked()), this, SLOT(slotMoveOn()));

		for (int trans_id = 0; trans_id < TranslateID::TRANS_END; ++trans_id) {
			ui_[id].trans[trans_id] = new QDoubleSpinBox();
			ui_[id].trans[trans_id]->setSingleStep(0.05);
			ui_[id].trans[trans_id]->setRange(-1000.0, 1000.0);
			ui_[id].trans[trans_id]->setValue(0.0);
			ui_[id].trans[trans_id]->setEnabled(false);
			prev_params_[id].trans[trans_id] = 0.0f;
			connect(ui_[id].trans[trans_id], SIGNAL(valueChanged(double)), this, SLOT(slotTranslate(double)));
		}

		for (int rotate_id = 0; rotate_id < RotateID::ROT_END; ++rotate_id) {
			ui_[id].rotate[rotate_id] = new QDoubleSpinBox();
			ui_[id].rotate[rotate_id]->setSingleStep(0.05);
			ui_[id].rotate[rotate_id]->setRange(-1000.0, 1000.0);
			ui_[id].rotate[rotate_id]->setValue(0.0);
			ui_[id].rotate[rotate_id]->setEnabled(false);
			prev_params_[id].rotate[rotate_id] = 0.0f;
			connect(ui_[id].rotate[rotate_id], SIGNAL(valueChanged(double)), this, SLOT(slotRotate(double)));
		}
		SetTextMoveToolButton((CutTypeID)id);
	}

	ui_[CutTypeID::MAXILLA].enable->setText(lang::LanguagePack::txt_maxillary());
	ui_[CutTypeID::MANDIBLE].enable->setText(lang::LanguagePack::txt_mandible());
	ui_[CutTypeID::CHIN].enable->setText(lang::LanguagePack::txt_chin());
}

void CephSurgeryTool::SetTextMoveToolButton(const surgery::CutTypeID& type) {
	ui_[type].move->setText(is_move_[type] ? lang::LanguagePack::txt_move_off() : lang::LanguagePack::txt_move_on());
}

void CephSurgeryTool::SetEnableCutItems(surgery::CutTypeID type, bool is_enable) {
	ui_[type].adjust->setEnabled(is_enable);
	ui_[type].move->setEnabled(is_enable);
	for (int trans_id = 0; trans_id < TranslateID::TRANS_END; ++trans_id)
		ui_[type].trans[trans_id]->setEnabled(is_enable);
	for (int rotate_id = 0; rotate_id < RotateID::ROT_END; ++rotate_id)
		ui_[type].rotate[rotate_id]->setEnabled(is_enable);
}

surgery::CutTypeID CephSurgeryTool::SenderID(QObject * sender) {
	int cut_id;
	for (cut_id = 0; cut_id < CutTypeID::CUT_TYPE_END; ++cut_id) {
		if (ui_[cut_id].enable == sender) {
			break;
		} else if (ui_[cut_id].adjust == sender) {
			break;
		} else if (ui_[cut_id].move == sender) {
			break;
		}
	}
	return static_cast<CutTypeID>(cut_id);
}

void CephSurgeryTool::slotEnable(int enable) {
	CutTypeID cut_id = SenderID(QObject::sender());
	bool checked = ui_[cut_id].enable->isChecked();

	slotAdjustOn();

	this->SetEnableCutItems(cut_id, checked);

	emit sigSurgeryMoveOn(cut_id, is_move_[cut_id]);
	emit sigSurgeryEnableOn(cut_id, checked);
}

void CephSurgeryTool::slotAdjustOn() {
	CutTypeID cut_id = SenderID(QObject::sender());
	bool checked = ui_[cut_id].adjust->isChecked();
	if (is_adjust_[cut_id]) {
		ui_[cut_id].adjust->setText(lang::LanguagePack::txt_adjust_cut());
		is_adjust_[cut_id] = false;
		ui_[cut_id].move->setEnabled(true);
	} else {
		ui_[cut_id].adjust->setText(lang::LanguagePack::txt_done());
		ui_[cut_id].move->setEnabled(false);
		is_adjust_[cut_id] = true;
	}
	emit sigSurgeryAdjustOn(cut_id, is_adjust_[cut_id]);
}

void CephSurgeryTool::slotMoveOn() {
	CutTypeID cut_id = SenderID(QObject::sender());
	bool checked = ui_[cut_id].move->isChecked();
	if (!is_adjust_[cut_id]) {
		is_move_[cut_id] = !is_move_[cut_id];
		SetTextMoveToolButton(cut_id);
		emit sigSurgeryMoveOn(cut_id, is_move_[cut_id]);
	}
}
