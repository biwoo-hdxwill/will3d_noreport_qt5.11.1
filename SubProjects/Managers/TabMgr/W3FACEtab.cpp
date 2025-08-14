#include "W3FACEtab.h"
/*=========================================================================

File:			class CW3FACEtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-12
Last date:		2016-01-14

=========================================================================*/
#include <QApplication>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QToolButton>

#include <Engine/Common/Common/event_handle_common.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/language_pack.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_face.h>
#endif

#include <Engine/UIModule/UITools/face_task_tool.h>
#include <Engine/UIModule/UITools/tool_mgr.h>

#include <Engine/UIModule/UIFrame/window_plane.h>
#include <Engine/UIModule/UIComponent/W3VTOSTO.h>
#include <Engine/UIModule/UIComponent/W3View3DCeph.h>

#include "W3FACEViewMgr.h"

CW3FACEtab::CW3FACEtab(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
	CW3VTOSTO* vtosto, CW3ResourceContainer* Rcontainer)
	: m_pgVTOSTO(vtosto),
	m_pgVREngine(VREngine),
	m_pgMPRengine(MPRengine),
	m_pgRcontainer(Rcontainer),
	task_tool_(new FaceTaskTool(this))
{
	ToolMgr::instance()->SetFaceTaskTool(task_tool_);

	if (m_pgVTOSTO->flag.loadTRD) task_tool_->EnableFaceUI();
}

CW3FACEtab::~CW3FACEtab(void)
{
	SAFE_DELETE_OBJECT(go_to_ceph_tab_widget_);
	SAFE_DELETE_OBJECT(m_pFACEViewMgr);
}
#ifndef WILL3D_VIEWER
void CW3FACEtab::exportProject(ProjectIOFace& out)
{
	if (m_pFACEViewMgr) m_pFACEViewMgr->exportProject(out);
}

void CW3FACEtab::importProject(ProjectIOFace& in)
{
	if (in.IsInit())
	{
		if (!initialized())
		{
			Initialize();
		}
		m_pFACEViewMgr->importProject(in);
	}
}
#endif
void CW3FACEtab::UpdateVRview(bool is_high_quality)
{
	m_pFACEViewMgr->UpdateVRview(is_high_quality);
}

void CW3FACEtab::SetCommonToolOnce(const common::CommonToolTypeOnce& type,
	bool on)
{
}

void CW3FACEtab::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	m_pFACEViewMgr->SetCommonToolOnOff(type);
}

void CW3FACEtab::DeleteMeasureUI(const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
	m_pFACEViewMgr->DeleteMeasureUI(view_type, measure_id);
}

void CW3FACEtab::ReleaseSurgeryViewFromLayout()
{
	if (tab_layout())
	{
		tab_layout()->removeWidget(m_pFACEViewMgr->getViewFaceAfter());
	}
}

void CW3FACEtab::SetLayout()
{
	if (!BaseTab::initialized())
		return;

	if (!m_pgVTOSTO->flag.calcDisp)
	{
		window_after_->SetView(go_to_ceph_tab_widget_);
		setFaceLayout(window_face_mesh_.get(), window_face_photo_.get(),
			window_before_.get(), window_after_.get());
	}
	else
	{
		if (m_pgVTOSTO->isAvailableFace())
		{
			emit sigRequestViewFaceAfter();

			window_after_->SetView(m_pFACEViewMgr->getViewFaceAfter());
			if (m_switchBeforeAfter)
			{
				setFaceLayout(window_before_.get(), window_after_.get());
			}
			else
			{
				setFaceLayout(window_face_mesh_.get(), window_face_photo_.get(),
					window_before_.get(), window_after_.get());
			}

			emit sigCephNoEventMode();
		}
		else
		{
			window_after_->SetView(go_to_ceph_tab_widget_);

			setFaceLayout(window_face_mesh_.get(), window_face_photo_.get(),
				window_before_.get(), window_after_.get());
		}
	}
}

void CW3FACEtab::setViewFaceAfter(QWidget* widget)
{
	bool is_visible_face = task_tool_->IsVisibleFace();
	m_pFACEViewMgr->setView3DFaceAfter(widget, is_visible_face);
}

void CW3FACEtab::Initialize()
{
	if (BaseTab::initialized())
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			"already initialized.");
		assert(false);
	}

	window_before_.reset(new WindowPlane(lang::LanguagePack::txt_before()));
	window_before_->setVisible(false);
	window_after_.reset(new WindowPlane(lang::LanguagePack::txt_after()));
	window_after_->setVisible(false);
	window_face_mesh_.reset(new WindowPlane(lang::LanguagePack::txt_face_model()));
	window_face_mesh_->setVisible(false);
	window_face_photo_.reset(new WindowPlane(lang::LanguagePack::txt_face_photo()));
	window_face_photo_->setVisible(false);

	m_pFACEViewMgr = new CW3FACEViewMgr(m_pgVREngine, m_pgMPRengine, m_pgVTOSTO,
		m_pgRcontainer);
	bool is_visible_face = task_tool_->IsVisibleFace();
	Qt::CheckState state =
		is_visible_face ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
	m_pFACEViewMgr->VisibleFace(state);

	emit sigRequestViewFaceAfter();

	CreateGoToCephTabWidget();
	go_to_ceph_tab_widget_->setVisible(false);

	window_before_->SetView(m_pFACEViewMgr->getViewFaceBefore());
	window_after_->SetView(go_to_ceph_tab_widget_);
	window_face_mesh_->SetView(m_pFACEViewMgr->getViewFaceMesh());
	window_face_photo_->SetView(m_pFACEViewMgr->getViewFacePhoto());
	BaseTab::set_initialized(true);

	SetLayout();

	connections();
}

void CW3FACEtab::connections()
{
	connect(task_tool_.get(), SIGNAL(sigFaceTask(FaceTaskID)), this, SLOT(slotFaceTask(FaceTaskID)));
	connect(task_tool_.get(), SIGNAL(sigFaceVisible(VisibleID, int)), this, SLOT(slotVisibleFace(VisibleID, int)));
	connect(task_tool_.get(), SIGNAL(sigFaceChangeFaceTransparency(float)), m_pFACEViewMgr, SLOT(slotFaceChangeFaceTransparency(float)));

	connect(go_to_ceph_tab_widget_, SIGNAL(clicked()), this, SLOT(slotGoToCephTab()));

	connect(m_pFACEViewMgr, SIGNAL(sigSave3DFaceToPLYFile()), this, SIGNAL(sigSave3DFaceToPLYFile()));
	connect(m_pFACEViewMgr, SIGNAL(sigSave3DFaceToOBJFile()), this, SIGNAL(sigSave3DFaceToOBJFile()));
	connect(m_pFACEViewMgr, &CW3FACEViewMgr::sigSetSoftTissueMin, this, &CW3FACEtab::sigSetSoftTissueMin);

#ifdef WILL3D_EUROPE
	connect(m_pFACEViewMgr, &CW3FACEViewMgr::sigShowButtonListDialog, this, &CW3FACEtab::sigShowButtonListDialog);
#endif // WILL3D_EUROPE
}

void CW3FACEtab::SetVisibleWindows(bool isVisible)
{
	if (!initialized()) return;

	if (isVisible)
		CW3LayoutFunctions::setVisibleWidgets(tab_layout_, isVisible);
	else
	{
		window_before_->setVisible(isVisible);
		window_after_->setVisible(isVisible);
		window_face_mesh_->setVisible(isVisible);
		window_face_photo_->setVisible(isVisible);
		setVisibleViews(isVisible);
	}
}

void CW3FACEtab::setVisibleViews(bool isVisible)
{
	CW3View3DCeph* view_face_after = (CW3View3DCeph*)m_pFACEViewMgr->getViewFaceAfter();

	if (isVisible)
	{
		if (m_pgVTOSTO->flag.calcDisp && m_pgVTOSTO->isAvailableFace() && view_face_after)
		{
			view_face_after->setViewType(common::ViewTypeID::FACE_AFTER);
			view_face_after->setVisible(isVisible);
		}
		else
		{
			go_to_ceph_tab_widget_->setVisible(isVisible);
		}
	}
	else
	{
		go_to_ceph_tab_widget_->setVisible(isVisible);
		if (view_face_after)
			view_face_after->setVisible(isVisible);
	}

	QWidget* view_face_before = m_pFACEViewMgr->getViewFaceBefore();
	QWidget* view_face_mesh = m_pFACEViewMgr->getViewFaceBefore();
	QWidget* view_face_photo = m_pFACEViewMgr->getViewFaceBefore();

	if (view_face_before)
		view_face_before->setVisible(isVisible);
	if (view_face_mesh)
		view_face_mesh->setVisible(isVisible);
	if (view_face_photo)
		view_face_photo->setVisible(isVisible);
}

void CW3FACEtab::slotGoToCephTab() { emit sigChangeTab(TabType::TAB_3DCEPH); }

void CW3FACEtab::slotFaceTask(const FaceTaskID& task_id)
{
	switch (task_id)
	{
	case FaceTaskID::GENERATE:
		TaskGenerateFace();
		break;
	case FaceTaskID::LOAD:
		TaskLoadPhoto();
		break;
	case FaceTaskID::MAPPING:
		TaskFaceMapping();
		break;
	case FaceTaskID::CLEAR:
		TaskClearMappingPoints();
		break;
	case FaceTaskID::COMPARE:
		TaskBeforeAndAfter();
		break;
	default:
		break;
	}
}

void CW3FACEtab::slotVisibleFace(const VisibleID& visible_id, int state)
{
	if (visible_id != VisibleID::FACEPHOTO) return;

	m_pFACEViewMgr->VisibleFace(state);
}

void CW3FACEtab::TaskLoadPhoto()
{
	QString file_name;
#if defined(__APPLE__)
	file_name = QFileDialog::getOpenFileName(
		nullptr, lang::LanguagePack::txt_file() + lang::LanguagePack::txt_open(),
		"/home", "Files (*.png *.jpg *.bmp *.trd)", nullptr,
		QFileDialog::Option::DontUseNativeDialog);
#else
	QSettings settings;
	file_name = QFileDialog::getOpenFileName(
		nullptr, lang::LanguagePack::txt_file() + lang::LanguagePack::txt_open(),
		settings.value("lastPath").toString(),
		tr("Files (*.png *.jpg *.bmp *.trd);"));
#endif
	if (file_name.isEmpty())
	{
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"CW3FACEtab::TaskLoadPhoto: failed to load image.");
		return;
	}

	if (file_name.right(3).toLower() == QString("png") ||
		file_name.right(3).toLower() == QString("jpg") ||
		file_name.right(3).toLower() == QString("bmp"))
	{
		m_pgVTOSTO->m_photoFilePath = file_name;
		emit m_pgVTOSTO->sigLoadPhoto(file_name);
	}
	else if (file_name.right(3).toLower() == QString("trd"))
	{
		if (m_pFACEViewMgr->LoadTRD(file_name))
		{
			task_tool_->EnableFaceUI();
			// TODO : send signal to MPR task tool to sync face resource exist
		}
	}
}

void CW3FACEtab::TaskGenerateFace()
{
	if (m_pFACEViewMgr->GenerateFace())
	{
		task_tool_->SyncVisibilityEnable(false);

		/*setVisibleViews(false);
		setLayout(is_otf_on_);
		emit sigChangeTab(TAB_FACESIM);*/
	}
}

void CW3FACEtab::TaskClearMappingPoints()
{
	m_pFACEViewMgr->ClearMappingPoints();
}

void CW3FACEtab::TaskFaceMapping()
{
	if (!m_pFACEViewMgr->FaceMapping())
	{
		return;
	}
	task_tool_->EnableFaceUI();
}

void CW3FACEtab::TaskBeforeAndAfter()
{
	if (m_pgVTOSTO->flag.doMapping || m_pgVTOSTO->flag.loadTRD)
	{
		if (m_pgVTOSTO->flag.calcDisp)
		{
			m_switchBeforeAfter = !m_switchBeforeAfter;

			SetVisibleWindows(false);
			SetLayout();
			const auto& event_common_handler =
				EventHandler::GetInstance()->GetCommonEventHandle();
			event_common_handler.EmitSigSetTabSlotLayout(tab_layout_);
			SetVisibleWindows(true);
		}
		else
		{
			task_tool_->SyncTaskUI(FaceTaskID::COMPARE, false);

			CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_27(),
				CW3MessageBox::Question);
			msgBox.setDetailedText(lang::LanguagePack::msg_50());
			if (msgBox.exec()) emit sigChangeTab(TAB_3DCEPH);
		}
	}
	else
	{
		task_tool_->SyncTaskUI(FaceTaskID::COMPARE, false);

		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_28(),
			CW3MessageBox::Information);
		msgBox.exec();
	}
}

bool CW3FACEtab::setFaceLayout(QWidget* v1, QWidget* v2, QWidget* v3,
	QWidget* v4)
{
	if (v1 == nullptr || v2 == nullptr || v3 == nullptr || v4 == nullptr)
		return false;

	if (tab_layout_)
	{
		CW3LayoutFunctions::RemoveWidgetsAll(tab_layout_);
		SAFE_DELETE_OBJECT(tab_layout_);
	}

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->setMargin(0);
	gridLayout->setSpacing(2);

	gridLayout->addWidget(v1, 0, 0);
	gridLayout->addWidget(v2, 0, 1);
	gridLayout->addWidget(v3, 1, 0);
	gridLayout->addWidget(v4, 1, 1);
	tab_layout_ = gridLayout;

	return true;
}

bool CW3FACEtab::setFaceLayout(QWidget* v1, QWidget* v2)
{
	if (v1 == nullptr || v2 == nullptr) return false;

	if (tab_layout_)
	{
		CW3LayoutFunctions::RemoveWidgetsAll(tab_layout_);
		SAFE_DELETE_OBJECT(tab_layout_);
	}
	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->setMargin(0);
	gridLayout->setSpacing(2);

	gridLayout->addWidget(v1, 0, 0);
	gridLayout->addWidget(v2, 0, 1);

	tab_layout_ = gridLayout;

	return true;
}

// load trd from willmaster
void CW3FACEtab::setTRDFromExternalProgram(const QString path,
	const bool onlyTRD)
{
	if (!initialized()) Initialize();

	if (m_pFACEViewMgr->setTRDFromExternalProgram(path, onlyTRD))
		task_tool_->EnableFaceUI();
}

QStringList CW3FACEtab::GetViewList()
{
	return QStringList{ window_face_mesh_.get()->window_title(),
					   window_face_photo_.get()->window_title(),
					   window_before_.get()->window_title(),
					   window_after_.get()->window_title() };
}

QImage CW3FACEtab::GetScreenshot(int view_type)
{
	QWidget* source = GetScreenshotSource(view_type);

	return BaseTab::GetScreenshot(source);
}

QWidget* CW3FACEtab::GetScreenshotSource(int view_type)
{
	QWidget* source = nullptr;

	switch (view_type)
	{
	case 1:
		source = window_face_mesh_.get();
		break;
	case 2:
		source = window_face_photo_.get();
		break;
	case 3:
		source = window_before_.get();
		break;
	case 4:
		if (!m_pgVTOSTO->flag.calcDisp)
			source = go_to_ceph_tab_widget_;
		else
			source = window_after_.get();
		break;
	}

	return source;
}

void CW3FACEtab::ApplyPreferences()
{
	if (m_pFACEViewMgr) m_pFACEViewMgr->ApplyPreferences();
}

#ifdef WILL3D_EUROPE
void CW3FACEtab::SyncControlButtonOut()
{
	m_pFACEViewMgr->SetSyncControlButtonOut();
}
#endif // WILL3D_EUROPE

void CW3FACEtab::CreateGoToCephTabWidget()
{
	SAFE_DELETE_OBJECT(go_to_ceph_tab_widget_);

	go_to_ceph_tab_widget_ = new QToolButton();
	go_to_ceph_tab_widget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->setSpacing(0);

	QLabel* arrow_image = new QLabel();
	arrow_image->setPixmap(QPixmap(":/image/back.png"));

	QLabel* message_label_1 = new QLabel();
	message_label_1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QFont font = message_label_1->font();
	font.setUnderline(true);

	message_label_1->setFont(font);
	message_label_1->setText(lang::LanguagePack::msg_92());
	//message_label_1->setWordWrap(true);
	message_label_1->setStyleSheet(
		QString(
			"QLabel"
			"{"
			"	font: 40px; font-style: bold; font-weight: 900;"
			"}"
		)
	);

	QLabel* message_label_2 = new QLabel();
	message_label_2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	message_label_2->setText(lang::LanguagePack::msg_93());
	//message_label_2->setWordWrap(true);
	message_label_2->setStyleSheet(
		QString(
			"QLabel"
			"{"
			"	font: 15px; font-style: bold; font-weight: 500;"
			"}"
		)
	);

	main_layout->addStretch(1);
	main_layout->addWidget(arrow_image, 0, Qt::AlignCenter);
	main_layout->addSpacing(20);
	main_layout->addWidget(message_label_1, 0, Qt::AlignCenter);
	main_layout->addSpacing(10);
	main_layout->addWidget(message_label_2, 0, Qt::AlignCenter);
	main_layout->addStretch(1);

	go_to_ceph_tab_widget_->setLayout(main_layout);
}
