#include "W3CephTracingBar.h"

#include <iostream>

#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QSignalMapper>
#include <QGridLayout>
#include <QScrollArea>
#include <QScrollBar>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/language_pack.h"
#include "../../../Managers/DBManager/W3DBM.h"

using std::exception;
using std::cout;
using std::endl;

CW3CephTracingBar::CW3CephTracingBar(QWidget* parent)
	:QFrame(parent) {
	this->setObjectName("CephTracingBar");
	CW3Theme* theme = CW3Theme::getInstance();

	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setAlignment(Qt::AlignTop);
	m_pMainLayout->setContentsMargins(0, 0, 0, 0);
	m_pMainLayout->setSpacing(0);

	QLabel* caption = new QLabel();
	caption->setContentsMargins(10, 0, 0, 0);
	caption->setMinimumHeight(30);
	caption->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	caption->setObjectName("Caption");
	caption->setText(lang::LanguagePack::txt_tracing());

	m_pCaptureImg = new QLabel();
	m_pCaptureImg->setPixmap(this->loadImage(":/image/tracing/guide/NoImage.png"));
	m_pCaptureImg->setMinimumSize(210, 190);
	m_pCaptureImg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pCaptureImg->setAlignment(Qt::AlignCenter);
	m_pCaptureImg->setObjectName("GuideImg");
	QVBoxLayout* listLayout = new QVBoxLayout();
	listLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	listLayout->setSpacing(0);
	listLayout->setContentsMargins(0, 0, 0, 0);
	listLayout->setAlignment(Qt::AlignTop);

	QLabel* listCaption = new QLabel(lang::LanguagePack::txt_tracing_list());
	listCaption->setContentsMargins(10, 0, 0, 0);
	listCaption->setMinimumHeight(30);
	listCaption->setObjectName("listCaption");
	listLayout->addWidget(listCaption);

	CW3DBM::getInstance()->getTracingtasksName(m_tracingLists);

	QSignalMapper* sigMapper = new QSignalMapper(this);

	m_pTracingListArea = new QScrollArea;
	m_pTracingListArea->setObjectName("listArea");
	m_pTracingListArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_pTracingListArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pTracingListArea->setWidgetResizable(true);
	m_pTracingListArea->setStyleSheet(theme->appQScrollBarStyleSheet());

	QWidget* scrollWidgetContents = new QWidget;
	m_pTracingListArea->setWidget(scrollWidgetContents);
	scrollWidgetContents->setLayout(listLayout);

	for (const auto& elem : m_tracingLists) {
		QToolButton* btnTracing = new QToolButton();
		btnTracing->setObjectName(tr("Tracing"));
		btnTracing->setMinimumWidth(210);
		btnTracing->setText(elem);
		btnTracing->setStyle(CW3Theme::getInstance()->cephTracingTaskToolStyle());

		connect(btnTracing, SIGNAL(clicked()), sigMapper, SLOT(map()));
		sigMapper->setMapping(btnTracing, elem);
		m_btnTracingLists[elem] = btnTracing;
		listLayout->addWidget(btnTracing);

		m_tracingStatus[elem] = TRACING_STATUS::TRACING_UNCHECK;
	}
	connect(sigMapper, SIGNAL(mapped(QString)), this, SLOT(slotClickedTracing(QString)));

	QFrame* dummy_command_ = new QFrame(this);
	dummy_command_->setObjectName("TracingCommand");
	QGridLayout* commandLayout = new QGridLayout();
	dummy_command_->setLayout(commandLayout);
	commandLayout->setAlignment(Qt::AlignHCenter);
	commandLayout->setContentsMargins(10, 10, 10, 10);
	commandLayout->setSpacing(10);
	for (int i = 0; i < COMMAND_END; i++)
	{
		QToolButton* btn = new QToolButton();
		btn->setStyleSheet(theme->appToolButtonStyleSheet());
		int k = i;
		commandLayout->addWidget(btn, static_cast<int>(k / 2), k % 2);
		m_btnCommand.push_back(btn);
	}

	m_btnCommand[COMMAND_START]->setText(lang::LanguagePack::txt_start());
	m_btnCommand[COMMAND_RESET]->setText(lang::LanguagePack::txt_reset());
	m_btnCommand[COMMAND_CANCEL]->setText(lang::LanguagePack::txt_cancel());
	m_btnCommand[COMMAND_SETUP]->setText(lang::LanguagePack::txt_setup());

	m_pMainLayout->addWidget(caption);
	m_pMainLayout->addWidget(m_pCaptureImg);
	m_pMainLayout->addWidget(m_pTracingListArea);
	m_pMainLayout->addWidget(dummy_command_);

#if CHINA_VERSION
	m_btnCommand[COMMAND_SETUP]->setVisible(false);
#endif //CHINA_VERSION

	this->setStyleSheet(CW3Theme::getInstance()->cephTracingBarStyleSheet());
	this->setLayout(m_pMainLayout);
	this->connections();

	this->setVisible(false);
}

CW3CephTracingBar::~CW3CephTracingBar() {}

void CW3CephTracingBar::checkAll() {
	setTracingStatus(m_tracingLists[0], TRACING_CHECK, ICON_TOP);
	setTracingStatus(m_tracingLists[m_tracingLists.size() - 1], TRACING_CHECK, ICON_BOTTOM);

	for (int i = 1; i < m_tracingLists.size() - 1; i++) {
		setTracingStatus(m_tracingLists[i], TRACING_CHECK, ICON_MID);
	}

	m_isSetCoordSys = true;
	m_isTracingDone = true;
}

void CW3CephTracingBar::awake() {
	if (m_isTracingDone) {
		m_pCaptureImg->setPixmap(this->loadImage(":/image/tracing/guide/NoImage.png"));
		return;
	}

	QString tracing;
	for (auto& elem : m_tracingStatus) {
		if (elem.second == TRACING_STATUS::TRACING_ACTIVE) {
			tracing = elem.first;
			break;
		}
	}

	if (tracing.isEmpty())
	{
		for (int i = 0; i < m_tracingLists.size(); i++)
		{
			if (m_tracingStatus[m_tracingLists[i]] == TRACING_UNCHECK)
			{
				this->setTracingStatus(m_tracingLists[i], TRACING_ACTIVE, ICON_ACTIVE);
				emit sigActiveTracingTask(m_tracingLists[i]);
				break;
			}
		}
	} else {
		this->setTracingStatus(tracing, TRACING_ACTIVE, ICON_ACTIVE);
		emit sigActiveTracingTask(tracing);
	}
}

void CW3CephTracingBar::slotNextTracingTask(const QString& checkTracing) {
	try {
		for (auto& elem : m_btnTracingLists)
			elem.second->setStyleSheet("QToolButton#Tracing { background-image: url(:/image/tracing/uncheck.png);}");

		if (m_tracingStatus.find(checkTracing) == m_tracingStatus.end())
            throw std::runtime_error("tracingStatus not found");

		m_tracingStatus[checkTracing] = TRACING_CHECK;

		std::vector<int> chkIndices;
		for (int i = 0; i < m_tracingLists.size(); i++)
		{
			if (m_tracingStatus[m_tracingLists[i]] == TRACING_CHECK)
			{
				chkIndices.push_back(i);
			} else if (chkIndices.size() > 1) {
				setTracingStatus(m_tracingLists[chkIndices.front()], TRACING_CHECK, ICON_TOP);
				setTracingStatus(m_tracingLists[chkIndices.back()], TRACING_CHECK, ICON_BOTTOM);

				for (int j = 1; j < chkIndices.size() - 1; j++)
					setTracingStatus(m_tracingLists[chkIndices[j]], TRACING_CHECK, ICON_MID);

				chkIndices.clear();
			} else if (chkIndices.size() == 1) {
				setTracingStatus(m_tracingLists[chkIndices.front()], TRACING_CHECK, ICON_ISOL);
				chkIndices.clear();
			}
		}

		if (chkIndices.size() == 1) {
			setTracingStatus(m_tracingLists[chkIndices.front()], TRACING_CHECK, ICON_ISOL);
			chkIndices.clear();
		}

		int idx = -1;

		for (int i = 0; i < m_tracingLists.size(); i++)
		{
			if (checkTracing == m_tracingLists[i])
			{
				idx = i;
				break;
			}
		}

		if (idx < 0)
            throw std::runtime_error("tracingList not found.");


		bool isSetCoordSys = true;
		QStringList lstCoordSys;

		for (int i = 0; i < m_tracingLists.size(); i++) {
			if (m_tracingLists[i].left(9) == "Coord_sys") {
				if (m_tracingStatus[m_tracingLists[i]] == TRACING_UNCHECK) {
					isSetCoordSys = false;
					break;
				} else
					lstCoordSys.push_back(m_tracingLists[i]);
			}
		}

		if (!m_isSetCoordSys && isSetCoordSys) {
			emit sigSetCoordSystem(lstCoordSys);
			m_isSetCoordSys = true;
		}

		QToolButton *curTask = nullptr;

		for (int i = idx + 1; i < m_tracingLists.size(); i++)
		{
			if (m_tracingStatus[m_tracingLists[i]] == TRACING_UNCHECK)
			{
				if (m_tracingLists[i].left(9) != "Coord_sys" && !isSetCoordSys)
					continue;

				curTask = m_btnTracingLists[m_tracingLists[i]];
				setTracingListScrollValue(curTask->pos().y());

				setTracingStatus(m_tracingLists[i], TRACING_ACTIVE, ICON_ACTIVE);
				emit sigActiveTracingTask(m_tracingLists[i]);
				return;
			}
		}

		for (int i = 0; i < m_tracingLists.size(); i++) {
			if (m_tracingStatus[m_tracingLists[i]] == TRACING_UNCHECK) {
				curTask = m_btnTracingLists[m_tracingLists[i]];
				setTracingListScrollValue(curTask->pos().y());

				setTracingStatus(m_tracingLists[i], TRACING_ACTIVE, ICON_ACTIVE);
				emit sigActiveTracingTask(m_tracingLists[i]);
				return;
			}
		}

		cout << "tracingFinished...." << endl;

		this->checkAll();

		//tracing이 끝나면 강제로 tracingbar 닫음.
		//if (!m_isTracingDone)
		{
			m_isTracingDone = true;
			emit sigSetCoordSystem(lstCoordSys);
			emit sigFinishedTracingTasks();
		}
    } catch (std::runtime_error& e) {
		cout << "CW3CephTracingBar::slotNextTracingTask: " << e.what() << endl;
	}
}

void CW3CephTracingBar::setTracingListScrollValue(const int& curTaskPos)
{
	QScrollBar *scrollBar = m_pTracingListArea->verticalScrollBar();

	int value = curTaskPos - (m_pTracingListArea->height() / 2);
	if (value < 0)
		value = 0;
	if (value > scrollBar->maximum())
		value = scrollBar->maximum();

	scrollBar->setValue(value);
}

void CW3CephTracingBar::slotSetGuideImage(const QString & landmark) {
	m_pCaptureImg->setPixmap(this->loadImage(":/image/tracing/guide/" + landmark + ".png"));
}

void CW3CephTracingBar::setTracingStatus(const QString& tracing, TRACING_STATUS status, TRACING_ICON_STYLE style) {
	try {
		if (m_tracingStatus.find(tracing) == m_tracingStatus.end())
            throw std::runtime_error("tracingStatus not found");

		if (m_btnTracingLists.find(tracing) == m_btnTracingLists.end())
            throw std::runtime_error("tracingList not found");

		m_tracingStatus[tracing] = status;

		switch (style) {
		case ICON_ACTIVE: m_btnTracingLists[tracing]->setStyleSheet("QToolButton#Tracing { background-image: url(:/image/tracing/active.png);}"); break;
		case ICON_BOTTOM: m_btnTracingLists[tracing]->setStyleSheet("QToolButton#Tracing { background-image: url(:/image/tracing/bottom.png);}"); break;
		case ICON_ISOL: m_btnTracingLists[tracing]->setStyleSheet("QToolButton#Tracing { background-image: url(:/image/tracing/isol.png);}"); break;
		case ICON_MID: m_btnTracingLists[tracing]->setStyleSheet("QToolButton#Tracing { background-image: url(:/image/tracing/mid.png);}"); break;
		case ICON_TOP: m_btnTracingLists[tracing]->setStyleSheet("QToolButton#Tracing { background-image: url(:/image/tracing/top.png);}"); break;
		case ICON_UNCHECK: m_btnTracingLists[tracing]->setStyleSheet("QToolButton#Tracing { background-image: url(:/image/tracing/uncheck.png);}"); break;
		}
	} catch (const exception& e) {
		cout << "CW3CephTracingBar::setTracingStatus: " << e.what() << endl;
	}
}
QPixmap CW3CephTracingBar::loadImage(const QString& path) {
	QPixmap pixmap;
	try {
		QImage img;
		if (img.load(path)) {
			pixmap = QPixmap::fromImage(img);
			img.scaled(210, 190, Qt::IgnoreAspectRatio);
			pixmap = pixmap.scaled(210, 190, Qt::IgnoreAspectRatio);
		} else {
            throw std::runtime_error("failed to load image.");
		}
	} catch (const exception& e) {
		cout << "CW3CephTracingBar::loadImage: " << e.what() << endl;
	}

	return pixmap;
}
void CW3CephTracingBar::slotClickedTracing(const QString& tracing) {
	if (!m_isSetCoordSys && tracing.left(9) != QString("Coord_sys")) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_03(), CW3MessageBox::Critical);
		msgBox.exec();

		return;
	}
	if (m_tracingStatus[tracing] == TRACING_STATUS::TRACING_CHECK) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_04(), CW3MessageBox::Question);
		msgBox.setDetailedText(lang::LanguagePack::msg_89());
		int ret = msgBox.exec();
	
		if (!ret)
			return;
	}

	this->setTracingStatus(tracing, TRACING_ACTIVE, ICON_ACTIVE);

	for (auto& elem : m_tracingStatus) {
		if (elem.second == TRACING_STATUS::TRACING_ACTIVE &&
			elem.first.compare(tracing) != 0)
			this->setTracingStatus(elem.first, TRACING_UNCHECK, ICON_UNCHECK);
	}

	emit sigActiveTracingTask(tracing);
}
void CW3CephTracingBar::slotClickedStart() {
	if (m_isTracingDone) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_05(), CW3MessageBox::Information);
		msgBox.setDetailedText(lang::LanguagePack::msg_53());
		int ret = msgBox.exec();
		return;
	}

	for (int i = 0; i < m_tracingLists.size(); i++)
		setTracingStatus(m_tracingLists[i], TRACING_UNCHECK, ICON_UNCHECK);

	m_pCaptureImg->setPixmap(this->loadImage(":/image/tracing/guide/NoImage.png"));

	m_isTracingDone = false;
	m_isSetCoordSys = false;

	emit sigClearTracingTasks();

	QString tracing;
	for (auto& elem : m_tracingStatus) {
		if (elem.second == TRACING_STATUS::TRACING_ACTIVE) {
			tracing = elem.first;
			break;
		}
	}

	if (tracing.isEmpty()) {
		for (int i = 0; i < m_tracingLists.size(); i++) {
			if (m_tracingStatus[m_tracingLists[i]] == TRACING_UNCHECK) {
				this->setTracingStatus(m_tracingLists[i], TRACING_ACTIVE, ICON_ACTIVE);
				emit sigActiveTracingTask(m_tracingLists[i]);
				break;
			}
		}
	} else {
		this->setTracingStatus(tracing, TRACING_ACTIVE, ICON_ACTIVE);
		emit sigActiveTracingTask(tracing);
	}

	m_pTracingListArea->verticalScrollBar()->setValue(0);
}
void CW3CephTracingBar::slotClickedReset() {
	if (m_isSetCoordSys || m_isTracingDone) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_06(), CW3MessageBox::Question);
		msgBox.setDetailedText(lang::LanguagePack::msg_48());
		int ret = msgBox.exec();

		if (!ret)
			return;
	}

	for (int i = 0; i < m_tracingLists.size(); i++)
		setTracingStatus(m_tracingLists[i], TRACING_UNCHECK, ICON_UNCHECK);

	m_pCaptureImg->setPixmap(this->loadImage(":/image/tracing/guide/NoImage.png"));

	m_isTracingDone = false;
	m_isSetCoordSys = false;

	emit sigClearTracingTasks();

	QString tracing;
	for (auto& elem : m_tracingStatus) {
		if (elem.second == TRACING_STATUS::TRACING_ACTIVE) {
			tracing = elem.first;
			break;
		}
	}

	if (tracing.isEmpty()) {
		for (int i = 0; i < m_tracingLists.size(); i++) {
			if (m_tracingStatus[m_tracingLists[i]] == TRACING_UNCHECK) {
				this->setTracingStatus(m_tracingLists[i], TRACING_ACTIVE, ICON_ACTIVE);
				emit sigActiveTracingTask(m_tracingLists[i]);
				break;
			}
		}
	} else {
		this->setTracingStatus(tracing, TRACING_ACTIVE, ICON_ACTIVE);
		emit sigActiveTracingTask(tracing);
	}

	m_pTracingListArea->verticalScrollBar()->setValue(0);
}

void CW3CephTracingBar::slotClickedCancel() 
{
	if (m_isSetCoordSys)
		emit sigCancelTracingTask();
}

void CW3CephTracingBar::slotClickedSetup() 
{
	//TODO..?
	emit sigSetupTracingTask();
}

void CW3CephTracingBar::connections() 
{
	connect(m_btnCommand[COMMAND_START], SIGNAL(clicked()), this, SLOT(slotClickedStart()));
	connect(m_btnCommand[COMMAND_RESET], SIGNAL(clicked()), this, SLOT(slotClickedReset()));
	connect(m_btnCommand[COMMAND_CANCEL], SIGNAL(clicked()), this, SLOT(slotClickedCancel()));
	connect(m_btnCommand[COMMAND_SETUP], SIGNAL(clicked()), this, SLOT(slotClickedSetup()));
}
