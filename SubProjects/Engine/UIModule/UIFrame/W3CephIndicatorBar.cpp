#include "W3CephIndicatorBar.h"

#include <cmath>

#include <QToolButton>
#include <QTabWidget>
#include <qboxlayout.h>
#include <QScrollArea>
#include <QComboBox>
#include <QSignalMapper>
#include <QLabel>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/language_pack.h"
#include "../../../Managers/DBManager/W3DBM.h"
#include "../../../Managers/DBManager/W3CephDM.h"

using std::cout;
using std::endl;
using glm::vec3;
using glm::vec4;

CW3CephIndicatorList::CW3CephIndicatorList(QString caption, QWidget* parent)
	: QWidget(parent) {
	m_pCaptionLayout = new QHBoxLayout();
	m_pCaptionLayout->setContentsMargins(10, 0, 0, 0);
	m_pCaptionLayout->setSpacing(0);

	m_pCaption = new QToolButton(this);
	m_pCaption->setObjectName("Caption");
	m_pCaption->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_strCaption = caption;
	m_strMinimize = QString("-    ");
	m_strMaximize = QString("+   ");

	m_pCaption->setText(m_strMinimize + m_strCaption);

	m_pSwitch = new QToolButton(this);
	m_pSwitch->setObjectName("Switch");
	m_pSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_strSwitchOn = QString("On");
	m_strSwitchOff = QString("Off");
	m_pSwitch->setText(m_strSwitchOn);

	m_pCaptionLayout->addWidget(m_pCaption);
	m_pCaptionLayout->addWidget(m_pSwitch);

	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setAlignment(Qt::AlignTop);
	m_pMainLayout->setContentsMargins(0, 0, 0, 0);
	m_pMainLayout->setSpacing(0);

	m_pContentLayout = new QVBoxLayout();
	m_pContentLayout->setContentsMargins(35, 0, 0, 0);
	m_pContentLayout->setSpacing(0);

	m_pMainLayout->addLayout(m_pCaptionLayout);
	m_pMainLayout->addLayout(m_pContentLayout);

	m_flagMinimize = false;
	m_sigMapper = new QSignalMapper(this);
	connect(m_sigMapper, SIGNAL(mapped(int)), this, SLOT(slotClickedContent(int)));
	this->setStyleSheet(CW3Theme::getInstance()->cephIndicatorListStyleSheet());
	this->setLayout(m_pMainLayout);
	this->connectMinimize();
	this->connectSwitch();
}
CW3CephIndicatorList::~CW3CephIndicatorList() {
	SAFE_DELETE_OBJECT(m_pMainLayout);
}
void CW3CephIndicatorList::connectMinimize() {
	connect(m_pCaption, SIGNAL(clicked()), this, SLOT(slotMinimize()));
}

void CW3CephIndicatorList::disconnectMinimize() {
	disconnect(m_pCaption, SIGNAL(clicked()), this, SLOT(slotMinimize()));
}

void CW3CephIndicatorList::connectSwitch() {
	connect(m_pSwitch, SIGNAL(clicked()), this, SLOT(slotSwitch()));
}

void CW3CephIndicatorList::disconnectSwitch() {
	disconnect(m_pSwitch, SIGNAL(clicked()), this, SLOT(slotSwitch()));
}

void CW3CephIndicatorList::setMinimizeText(const QString & str) {
	m_strMinimize = str;
	if (!m_flagMinimize) {
		m_pCaption->setText(m_strMinimize + m_strCaption);
	}
}

void CW3CephIndicatorList::setMaximizeText(const QString & str) {
	m_strMaximize = str;
	if (m_flagMinimize) {
		m_pCaption->setText(m_strMaximize + m_strCaption);
	}
}

void CW3CephIndicatorList::setSwitchTextOn(const QString & str) {
	if (m_pSwitch->text() == m_strSwitchOn) {
		m_pSwitch->setText(str);
	}
	m_strSwitchOn = str;
}

void CW3CephIndicatorList::setSwitchTextOff(const QString & str) {
	if (m_pSwitch->text() == m_strSwitchOff) {
		m_pSwitch->setText(str);
	}
	m_strSwitchOff = str;
}

void CW3CephIndicatorList::changeAnalysisMode() {
	m_pSwitch->setVisible(false);

	QLabel* value = new QLabel(this);
	QLabel* norm = new QLabel(this);
	QLabel* sd = new QLabel(this);

	value->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	norm->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	value->setMinimumWidth(46);
	value->setText("Value");

	norm->setMinimumWidth(46);
	norm->setText("Norm");

	sd->setText("S.D");
	this->setStyleSheet(CW3Theme::getInstance()->cephIndicatorListStyleSheet(true));
	m_pCaptionLayout->addWidget(value);
	m_pCaptionLayout->addWidget(norm);
	m_pCaptionLayout->addWidget(sd);
}

QStringList CW3CephIndicatorList::getContentStrList() {
	QStringList strList;
	for (const auto& elem : m_contents) {
		strList.push_back(elem->text());
	}
	return strList;
}

void CW3CephIndicatorList::setSwitch(bool isSwitchOn) {
	m_pSwitch->setText(isSwitchOn ? m_strSwitchOn : m_strSwitchOff);
	setContentSwitch(isSwitchOn);
	m_flagSwitch = isSwitchOn;
	this->update();
}

void CW3CephIndicatorList::syncStatus() {
	for (int i = 0; i < m_contents.size(); i++) {
		QString str = m_contents[i]->text();
		QStringList strList = str.split("\n");
		int nEndIdx = strList.size() - 1;

		if (strList[nEndIdx] == QString("On")) {
			emit sigChangeContentSwitch(m_strCaption, strList[0], true);
		} else {
			emit sigChangeContentSwitch(m_strCaption, strList[0], false);
		}
	}
}

void CW3CephIndicatorList::slotMinimize() {
	if (m_flagMinimize) {
		m_pCaption->setText(m_strMaximize + m_strCaption);

		for (const auto& elem : m_contents) {
			elem->setVisible(false);
		}
		m_flagMinimize = false;
	} else {
		m_pCaption->setText(m_strMinimize + m_strCaption);

		for (const auto& elem : m_contents) {
			elem->setVisible(true);
		}
		m_flagMinimize = true;
	}
	this->update();
}
void CW3CephIndicatorList::setContentSwitch(bool isEnable) {
	for (int i = 0; i < m_contents.size(); i++) {
		QString str = m_contents[i]->text();
		QStringList strList = str.split("\n");
		int nEndIdx = strList.size() - 1;

		if (strList[nEndIdx] == QString("On") || strList[nEndIdx] == QString("Off")) {
			if (isEnable) {
				strList[nEndIdx] = QString("On");

				QString text;
				for (int j = 0; j < strList.size(); j++) {
					text += strList[j] + "\n";
				}
				text = text.left(text.size() - 1);
				m_contents[i]->setText(text);

				emit sigChangeContentSwitch(m_strCaption, strList[0], true);
			} else {
				strList[nEndIdx] = QString("Off");

				QString text;
				for (int j = 0; j < strList.size(); j++) {
					text += strList[j] + "\n";
				}
				text = text.left(text.size() - 1);
				m_contents[i]->setText(text);

				emit sigChangeContentSwitch(m_strCaption, strList[0], false);
			}
		}
	}
}
void CW3CephIndicatorList::slotSwitch() {
	if (m_pSwitch->text() == m_strSwitchOn) {
		m_pSwitch->setText(m_strSwitchOff);
		setContentSwitch(false);
	} else {
		m_pSwitch->setText(m_strSwitchOn);
		setContentSwitch(true);
	}
	this->update();
}

void CW3CephIndicatorList::slotClickedContent(int nIndex) {
	try {
		if (nIndex >= m_contents.size())
			throw std::runtime_error("out of range");

		QString str = m_contents[nIndex]->text();
		QStringList strList = str.split("\n");
		int nEndIdx = strList.size() - 1;

		if (strList[nEndIdx] == QString("On")) {
			strList[nEndIdx] = QString("Off");

			QString text;
			for (int i = 0; i < strList.size(); i++) {
				text += strList[i] + "\n";
			}
			text = text.left(text.size() - 1);
			m_contents[nIndex]->setText(text);

			emit sigChangeContentSwitch(m_strCaption, strList[0], false);
		} else if (strList[nEndIdx] == QString("Off")) {
			strList[nEndIdx] = QString("On");

			QString text;
			for (int i = 0; i < strList.size(); i++) {
				text += strList[i] + "\n";
			}
			text = text.left(text.size() - 1);
			m_contents[nIndex]->setText(text);

			emit sigChangeContentSwitch(m_strCaption, strList[0], true);
		}
	} catch (std::runtime_error &e) {
		cout << "CW3CephIndicatorList::slotClickedContent: " << e.what() << endl;
	}
}

void CW3CephIndicatorList::addRow(const QStringList& strColumns) {
	try {
		if (strColumns.size() == 0)
			throw std::runtime_error("strColumns size is zero.");

		for (auto & elem : m_contents) {
			QStringList lstStr = elem->text().split("\n");
			if (lstStr[0] == strColumns[0]) {
				if (lstStr.back() == m_strSwitchOff ||
					lstStr.back() == m_strSwitchOn) {
					QString strSwitch = lstStr.back();
					lstStr = strColumns;
					lstStr[strColumns.size() - 1] = strSwitch;
				} else {
					lstStr = strColumns;
				}

				QString text;
				for (int i = 0; i < lstStr.size(); i++) {
					text += lstStr[i] + "\n";
				}
				text = text.left(text.size() - 1);
				elem->setText(text);

				return; // escape function.
			}
		}

		QToolButton* btnTool = new QToolButton(this);
		btnTool->setObjectName("Content");
		btnTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_pContentLayout->addWidget(btnTool);

		QString text;
		for (int i = 0; i < strColumns.size(); i++) {
			text += strColumns[i] + "\n";
		}
		text = text.left(text.size() - 1);
		btnTool->setText(text);
		connect(btnTool, SIGNAL(clicked()), m_sigMapper, SLOT(map()));
		m_contents.push_back(btnTool);
		m_sigMapper->setMapping(btnTool, m_contents.size() - 1);
	} catch (std::runtime_error& e) {
		cout << "CW3CephIndicatorList::addRow: " << e.what() << endl;
	}
}

CW3CephAnalysisGraph::CW3CephAnalysisGraph(QWidget* parent)
	: QWidget(parent) {
	this->setMaximumWidth(130);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_polyValues = nullptr;
}
CW3CephAnalysisGraph::~CW3CephAnalysisGraph() {
}

void CW3CephAnalysisGraph::addAnalysisList(CW3CephIndicatorList* analysisList) {
	try {
		RECORD rec; //for splitter
		rec.isCaption = true;
		m_records.push_back(rec);

		QStringList contents = analysisList->getContentStrList();
		for (const auto& cont : contents) {
			QStringList values = cont.split("\n");

			RECORD rec;
			if (values.size() != 4)
				rec = { 0.0, 0.0, 0.0, false, false };
			else
				rec = { values[1].toFloat(), values[2].toFloat(), values[3].toFloat(), true, false };

			m_records.push_back(rec);
		}
	} catch (std::runtime_error& e) {
		cout << "CW3CephAnalysisGraph::addAnalysisList: " << e.what() << endl;
	}
}

void CW3CephAnalysisGraph::clear() {
	m_records.clear();
}

void CW3CephAnalysisGraph::paintEvent(QPaintEvent * event) {
	QPainter painter(this);
	this->drawScalemark(&painter);
	painter.setRenderHint(QPainter::Antialiasing);
	this->drawValueLines(&painter);
	QWidget::paintEvent(event);
}

void CW3CephAnalysisGraph::drawScalemark(QPainter* painter) {
	painter->save();
	painter->setPen(QPen(QBrush(QColor("#FF8C8C8C")), 1));
	QFont font = painter->font();
	font.setPixelSize(8);
	painter->setFont(font);
	float width = static_cast<float>(this->rect().width());
	float height = static_cast<float>(this->rect().height());

	float mid = width * 0.5f;
	float currY = -30.0f;

	QVector<QPointF> polyValues;
	for (const auto& elem : m_records) {
		if (elem.isCaption) {
			polyValues.push_back(QPointF(mid, currY));
			currY += 30.0f;
			polyValues.push_back(QPointF(mid, currY));
		} else {
			float posY = currY + 12.5f;

			if (elem.isUsed) {
				float value;

				if (isnan(elem.value))
					value = elem.sd;
				else
					value = elem.value;

				painter->drawLine(0, posY, width, posY);

				float scaleToValue = elem.sd / 12.5f;
				float range = scaleToValue * width;
				float stride = range / 5.0f;
				float xoffset = width / 5.0f;
				for (int i = 1; i < 5; i++) {
					float xPos = xoffset * i;
					painter->drawText(QPointF(xPos - 3.0, posY - 4.0), QString("%1").arg(static_cast<int>(elem.norm + stride * i - range * 0.5f)));
					painter->drawLine(xPos, posY - 2.5, xPos, posY + 2.5);
				}

				painter->drawLine(0.0f, posY - 2.5f, 0.0f, posY + 2.5f);
				painter->drawLine(width - 1.0f, posY - 2.5f, width - 1.0f, posY + 2.5f);

				float valuePosX = (1.0f / scaleToValue)*(value - elem.norm) + mid;

				polyValues.push_back(QPointF(valuePosX, posY));
			} else {
				polyValues.push_back(QPointF(mid, posY));
			}
			currY += 25.0f;
		}
	}

	polyValues.push_back(QPointF(mid, height));

	if (m_polyValues != nullptr) {
		SAFE_DELETE_OBJECT(m_polyValues);
	}

	m_polyValues = new QPolygonF(polyValues);

	painter->restore();
}

void CW3CephAnalysisGraph::drawValueLines(QPainter * painter) {
	float width = static_cast<float>(this->rect().width());
	float height = static_cast<float>(this->rect().height());

	float mid = width * 0.5f;

	painter->save();
	{
		painter->setPen(QPen(QBrush(QColor("#82ABFF")), 1));
		const QPointF points[7] = {
			QPointF(mid, 0),
			QPointF(mid - 12.5f, 12.5f),
			QPointF(mid - 12.5f, height - 12.5f),
			QPointF(mid, height),
			QPointF(mid + 12.5f, height - 12.5f),
			QPointF(mid + 12.5f, 12.5f),
			QPointF(mid, 0)
		};
		painter->drawPolyline(points, 7);

		QRect rect = this->rect();
		QLinearGradient gradient(rect.x(), 0, rect.width(), 0);

		gradient.setColorAt(0.0, QColor("#FFB50B0B"));
		gradient.setColorAt(0.30, QColor("#FFFF8785"));
		gradient.setColorAt(0.40, QColor("#FFFFFE85"));
		gradient.setColorAt(0.45, QColor("#FF85DA85"));
		gradient.setColorAt(0.5, QColor("#FF85DA85"));
		gradient.setColorAt(0.55, QColor("#FF85DA85"));
		gradient.setColorAt(0.60, QColor("#FFFFFE85"));
		gradient.setColorAt(0.70, QColor("#FFFF8785"));
		gradient.setColorAt(1, QColor("#FFB50B0B"));

		//gradient.setColorAt(0.0, QColor("#FFB50B0B"));
		//gradient.setColorAt(0.30, QColor("#FFFF8785"));
		//gradient.setColorAt(0.40, QColor("#FF85DA85"));
		//gradient.setColorAt(0.45, QColor("#FFFFFE85"));
		//gradient.setColorAt(0.5, QColor("#FFFFFE85"));
		//gradient.setColorAt(0.55, QColor("#FFFFFE85"));
		//gradient.setColorAt(0.60, QColor("#FF85DA85"));
		//gradient.setColorAt(0.70, QColor("#FFFF8785"));
		//gradient.setColorAt(1, QColor("#FFB50B0B"));

		painter->setBrush(gradient);
		painter->setPen(QPen(QBrush(QColor("#FFFF4646")), 1));
		painter->drawPolygon(*m_polyValues);

		painter->setPen(QPen(QBrush(QColor("#FEFF9E")), 1));
		painter->drawLine(mid, 0.0, mid, height);
	}painter->restore();
}

CW3CephIndicatorBar::CW3CephIndicatorBar(CW3CephDM* DataManager, QWidget* parent)
	: m_pgDataManager(DataManager), QFrame(parent) {
	this->setObjectName("CephIndicatorBar");
	CW3Theme* theme = CW3Theme::getInstance();
	QVBoxLayout* mainLayout = new QVBoxLayout();
	this->setMinimumWidth(448);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->setSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	this->setLayout(mainLayout);

	m_pTabWidget = new QTabWidget(this);
	mainLayout->addWidget(m_pTabWidget);

	m_pTabWidget->setTabPosition(QTabWidget::North);

	for (int i = 0; i < TAB_END; i++) {
		QWidget* tab = new QWidget(this);
		QVBoxLayout* dummyLayout = new QVBoxLayout;
		dummyLayout->setAlignment(Qt::AlignTop);
		dummyLayout->setContentsMargins(0, 0, 0, 0);
		dummyLayout->setMargin(0);
		tab->setLayout(dummyLayout);
		m_pTabs.push_back(tab);

		QScrollArea* tabScrollArea = new QScrollArea;
		tabScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		tabScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		tabScrollArea->setWidgetResizable(true);
		tabScrollArea->setStyleSheet(theme->appQScrollBarStyleSheet());

		QWidget* scrollWidgetContents = new QWidget;
		tabScrollArea->setWidget(scrollWidgetContents);
		dummyLayout->addWidget(tabScrollArea);

		CW3CephIndicatorTabStyle* tabStyle = new CW3CephIndicatorTabStyle();
		m_pTabStyle.push_back(tabStyle);

		if (i == TAB_ANALYSIS) {
			QVBoxLayout* vLayout = new QVBoxLayout;
			scrollWidgetContents->setLayout(vLayout);
			vLayout->setSpacing(5);
			vLayout->setContentsMargins(0, 0, 0, 0);

			QVBoxLayout* vComboLayout = new QVBoxLayout;
			vComboLayout->setContentsMargins(8, 8, 8, 0);
			m_pSelectAnalysis = new QComboBox(this);
			m_pSelectAnalysis->setStyle(m_pTabStyle[TAB_ANALYSIS]);
			m_pSelectAnalysis->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			std::map<QString, QStringList> analyInfo;
			CW3DBM::getInstance()->getAnalysisAll(analyInfo);

			int analIdx = 0;
			for (const auto& elem : analyInfo) {
				m_pSelectAnalysis->addItem(elem.first);

				if (elem.first == "Default") //temporary... 환경설정에서 init analysis 기능 추가할 때 변경
					m_pSelectAnalysis->setCurrentIndex(analIdx);

				analIdx++;
			}

			m_pSelectAnalysis->setObjectName("SelectAnalysis");
			m_pSelectAnalysis->setVisible(false);

			connect(m_pSelectAnalysis, SIGNAL(currentTextChanged(QString)), this, SLOT(slotSelectedAnalysis(QString)));
			vComboLayout->addWidget(m_pSelectAnalysis);

			QHBoxLayout* hLayout = new QHBoxLayout;
			hLayout->setSpacing(0);
			hLayout->setContentsMargins(0, 0, 0, 0);

			vLayout->addLayout(vComboLayout);
			vLayout->addLayout(hLayout);

			QVBoxLayout* tabLayout = new QVBoxLayout;
			tabLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
			tabLayout->setSpacing(0);
			tabLayout->setContentsMargins(0, 0, 0, 0);
			tabLayout->setAlignment(Qt::AlignTop);

			hLayout->addLayout(tabLayout);

			QVBoxLayout* graphLayout = new QVBoxLayout;
			graphLayout->setAlignment(Qt::AlignTop);
			graphLayout->setContentsMargins(0, 3, 7, 0);
			graphLayout->setSpacing(6);
			m_pAnalysisGraph = new CW3CephAnalysisGraph();
			m_pAnalysisGraph->setVisible(false);

			m_pRaceComboBox = new QComboBox(this);
			m_pRaceComboBox->setObjectName("RaceComboBox");
			m_pRaceComboBox->setStyle(m_pTabStyle[TAB_ANALYSIS]);
			m_pRaceComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
			m_pRaceComboBox->setVisible(false);

			QStringList races;
			CW3DBM::getInstance()->getNormDataRace(races);
			for (int i = 0; i < races.size(); i++) {
				m_pRaceComboBox->insertItem(i, races[i]);
				if (races[i] == "Korean")
					m_pRaceComboBox->setCurrentIndex(i);
			}
			connect(m_pRaceComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(slotChangedRaceBox(QString)));
			graphLayout->addWidget(m_pRaceComboBox);
			graphLayout->addWidget(m_pAnalysisGraph);
			hLayout->addLayout(graphLayout);

			m_pTabLayouts.push_back(tabLayout);
		} else {
			QVBoxLayout* tabLayout = new QVBoxLayout;
			scrollWidgetContents->setLayout(tabLayout);
			tabLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
			tabLayout->setSpacing(0);
			tabLayout->setContentsMargins(5, 5, 5, 5);
			tabLayout->setAlignment(Qt::AlignTop);

			m_pTabLayouts.push_back(tabLayout);
		}
	}

	m_pTabStyle[TAB_ANALYSIS]->setTabStyle(CW3CephIndicatorTabStyle::ANALYSIS);
	m_pTabStyle[TAB_REFERENCE]->setTabStyle(CW3CephIndicatorTabStyle::REFERENCE);
	m_pTabStyle[TAB_MEASUREMENT]->setTabStyle(CW3CephIndicatorTabStyle::MEASUREMENT);
	m_pTabStyle[TAB_LANDMARK]->setTabStyle(CW3CephIndicatorTabStyle::LANDMARK);

	m_pTabWidget->addTab(m_pTabs[TAB_LANDMARK], lang::LanguagePack::txt_landmark());
	m_pTabWidget->addTab(m_pTabs[TAB_MEASUREMENT], lang::LanguagePack::txt_measure());
	m_pTabWidget->addTab(m_pTabs[TAB_REFERENCE], lang::LanguagePack::txt_reference());
	m_pTabWidget->addTab(m_pTabs[TAB_ANALYSIS], lang::LanguagePack::txt_analysis());

	this->setStyleSheet(CW3Theme::getInstance()->cephIndicatorBarStyleSheet());

	connect(m_pgDataManager, SIGNAL(sigTurnOnEnable()), this, SLOT(slotTurnOnEnableCephData()));
	connect(m_pgDataManager, SIGNAL(sigUpdateLandmarkPositions()), this, SLOT(slotUpdateLandmarkPositions()));

	setVisible(false);
}

CW3CephIndicatorBar::~CW3CephIndicatorBar() {
	this->releaseIndicatorList();

	SAFE_DELETE_OBJECT(m_pRaceComboBox);
	SAFE_DELETE_OBJECT(m_pAnalysisGraph);

	while (m_pTabStyle.size()) {
		auto iter = m_pTabStyle.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_pTabStyle.erase(iter);
	}

	while (m_pTabLayouts.size()) {
		auto iter = m_pTabLayouts.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_pTabLayouts.erase(iter);
	}

	while (m_pTabs.size()) {
		auto iter = m_pTabs.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_pTabs.erase(iter);
	}

	SAFE_DELETE_OBJECT(m_pTabWidget);
}

CW3CephIndicatorBar::TAB CW3CephIndicatorBar::getCurrentTab() {
	return static_cast<TAB>(m_pTabWidget->currentIndex());
}

void CW3CephIndicatorBar::setCurrentTab(TAB tab) {
	m_pTabWidget->setCurrentIndex(static_cast<int>(tab));
}

void CW3CephIndicatorBar::releaseIndicatorList() {
	this->releaseLandmark();
	this->releaseMeasurement();
	this->releaseReference();
	this->releaseAnalysis();
}
void CW3CephIndicatorBar::releaseLandmark() {
	while (m_landmarkList.size()) {
		auto iter = m_landmarkList.begin();
		SAFE_DELETE_OBJECT(iter->second);
		m_landmarkList.erase(iter);
	}
}
void CW3CephIndicatorBar::releaseMeasurement() {
	while (m_measurementList.size()) {
		auto iter = m_measurementList.begin();
		SAFE_DELETE_OBJECT(iter->second);
		m_measurementList.erase(iter);
	}
}
void CW3CephIndicatorBar::releaseAnalysis() {
	while (m_analysisList.size()) {
		auto iter = m_analysisList.begin();
		SAFE_DELETE_OBJECT(iter->second);
		m_analysisList.erase(iter);
	}
	if (m_pSelectAnalysis)
		m_pSelectAnalysis->setVisible(false);
	if (m_pAnalysisGraph)
		m_pAnalysisGraph->setVisible(false);
	if (m_pRaceComboBox)
		m_pRaceComboBox->setVisible(false);
}
void CW3CephIndicatorBar::releaseReference() {
	while (m_referenceList.size()) {
		auto iter = m_referenceList.begin();
		SAFE_DELETE_OBJECT(iter->second);
		m_referenceList.erase(iter);
	}
}
void CW3CephIndicatorBar::updateMeasureAndAnalysis(const QString & analysis) {
	QList<QStringList> measurement;
	CW3DBM::getInstance()->getMeasurementNameFromAnalysis(analysis, measurement);

	for (const auto & elem : measurement) {
		QString name = elem.at(0);

		QString group = elem.at(1);
		QString type = elem.at(2);
		QString proj = elem.at(3);
		if (m_measurementList.find(group) == m_measurementList.end()) {
			m_measurementList[group] = new CW3CephIndicatorList(group);
			m_pTabStyle[TAB_MEASUREMENT]->setColumnWidth(0, 190);
			m_pTabStyle[TAB_MEASUREMENT]->setColumnWidth(1, 100);
			m_pTabStyle[TAB_MEASUREMENT]->setColumnWidth(2, 70);
			m_measurementList[group]->setStyle(m_pTabStyle[TAB_MEASUREMENT]);
			m_pTabLayouts[TAB_MEASUREMENT]->addWidget(m_measurementList[group]);
			connect(m_measurementList[group], SIGNAL(sigChangeContentSwitch(QString, QString, bool)),
					this, SIGNAL(sigMeasurementChangeContentSwitch(QString, QString, bool)));
		}

		QStringList columns;
		columns.push_back(name);

		if (type == "Angle")
			columns.push_back("degree*");
		else
			columns.push_back("mm*");
		QString value = QString("%1").arg(m_pgDataManager->getMeasurementValue(name), 0, 'f', 1);

		columns.push_back(value);
		columns.push_back("On");

		m_measurementList[group]->addRow(columns);

		if (m_analysisList.find(group) == m_analysisList.end()) {
			m_analysisList[group] = new CW3CephIndicatorList(group);
			m_analysisList[group]->disconnectMinimize();
			m_analysisList[group]->disconnectSwitch();
			m_analysisList[group]->setSwitchTextOn("");
			m_analysisList[group]->changeAnalysisMode();

			m_pTabStyle[TAB_ANALYSIS]->setColumnWidth(0, 145);
			m_pTabStyle[TAB_ANALYSIS]->setColumnWidth(1, 45);
			m_pTabStyle[TAB_ANALYSIS]->setColumnWidth(2, 45);
			m_analysisList[group]->setStyle(m_pTabStyle[TAB_ANALYSIS]);
			m_pTabLayouts[TAB_ANALYSIS]->addWidget(m_analysisList[group]);
		}

		float mean, sd;

		columns.clear();
		if (CW3DBM::getInstance()->getNormDataRecordFromMeasurement(name, m_pRaceComboBox->currentText(), mean, sd)) {
			columns.push_back(name);
			columns.push_back(value);
			columns.push_back(QString("%1").arg(mean, 0, 'f', 2));
			columns.push_back(QString("%1").arg(sd, 0, 'f', 2));

			m_analysisList[group]->addRow(columns);
		} else {
			columns.push_back(name);
			columns.push_back(value);
			m_analysisList[group]->addRow(columns);
		}
	}

	for (const auto& elem : m_measurementList) {
		elem.second->setSwitch(false);
	}

	m_pAnalysisGraph->clear();
	for (int i = 0; i < m_pTabLayouts[TAB_ANALYSIS]->count(); i++) {
		m_pAnalysisGraph->addAnalysisList((dynamic_cast<CW3CephIndicatorList*>(m_pTabLayouts[TAB_ANALYSIS]->itemAt(i)->widget())));
	}
	m_pAnalysisGraph->setMaximumHeight((measurement.size()) * 25 + (m_analysisList.size() - 1) * 30);
	m_pSelectAnalysis->setVisible(true);
	m_pRaceComboBox->setVisible(true);
	m_pAnalysisGraph->setVisible(true);
	m_pAnalysisGraph->update();
}

void CW3CephIndicatorBar::updateLandmarkAndReference() {
	std::map<QString, glm::vec3> actualLandmarks = m_pgDataManager->getActualLandmarks();

	if (actualLandmarks.size() != 0) {
		CW3DBM* dbm = CW3DBM::getInstance();
		for (const auto& elem : actualLandmarks) {
			QString group;
			dbm->getLandmarkGroupFromLandmark(elem.first, group);
			if (m_landmarkList.find(group) == m_landmarkList.end()) {
				m_landmarkList[group] = new CW3CephIndicatorList(group);
				m_pTabStyle[TAB_LANDMARK]->setColumnWidth(0, 170);
				m_pTabStyle[TAB_LANDMARK]->setColumnWidth(1, 190);
				m_landmarkList[group]->setStyle(m_pTabStyle[TAB_LANDMARK]);
				m_pTabLayouts[TAB_LANDMARK]->addWidget(m_landmarkList[group]);
				connect(m_landmarkList[group], SIGNAL(sigChangeContentSwitch(QString, QString, bool)),
						this, SIGNAL(sigLandmarkChangeContentSwitch(QString, QString, bool)));
			}

			QString pos = QString("(%1, %2, %3)")
				.arg(elem.second.x, 0, 'f', 1)
				.arg(elem.second.y, 0, 'f', 1)
				.arg(elem.second.z, 0, 'f', 1);

			QStringList columns;
			columns.push_back(elem.first);
			columns.push_back(pos);
			columns.push_back("On");

			m_landmarkList[group]->addRow(columns);
		}

		for (const auto& elem : m_landmarkList) {
			elem.second->setSwitch(false);
		}

		QStringList rplanes;
		dbm->getReferenceName(rplanes);

		for (const auto& elem : rplanes) {
			QString group = lang::LanguagePack::txt_reference_planes();
			if (m_referenceList.find(group) == m_referenceList.end()) {
				m_referenceList[group] = new CW3CephIndicatorList(group);
				m_pTabStyle[TAB_REFERENCE]->setColumnWidth(0, 360);
				m_referenceList[group]->setStyle(m_pTabStyle[TAB_REFERENCE]);
				m_pTabLayouts[TAB_REFERENCE]->addWidget(m_referenceList[group]);
				connect(m_referenceList[group], SIGNAL(sigChangeContentSwitch(QString, QString, bool)),
						this, SIGNAL(sigReferenceChangeContentSwitch(QString, QString, bool)));
			}

			if (m_pgDataManager->getReferencePlane(elem) != vec4(0.0f)) {
				QStringList columns = { elem , "On" };
				m_referenceList[group]->addRow(columns);
			}
		}

		for (const auto& elem : m_referenceList) {
			elem.second->setSwitch(false);
		}
	}
}

void CW3CephIndicatorBar::TracingTasksClear() {
	this->releaseIndicatorList();
}

void CW3CephIndicatorBar::slotSelectedAnalysis(const QString& analysis) {
	this->releaseMeasurement();
	this->releaseAnalysis();

	if (m_pgDataManager->isSetCoordSystem())
		updateMeasureAndAnalysis(analysis);
}

void CW3CephIndicatorBar::slotSyncCephView() {
	for (const auto& elem : m_landmarkList)
		elem.second->syncStatus();

	for (const auto& elem : m_measurementList)
		elem.second->syncStatus();

	for (const auto& elem : m_analysisList)
		elem.second->syncStatus();

	for (const auto& elem : m_referenceList)
		elem.second->syncStatus();
}

void CW3CephIndicatorBar::slotChangedRaceBox(const QString& race) {
	this->releaseMeasurement();
	this->releaseAnalysis();

	if (m_pgDataManager->isSetCoordSystem())
		updateMeasureAndAnalysis(m_pSelectAnalysis->currentText());
}

void CW3CephIndicatorBar::slotTurnOnEnableCephData() {
	this->releaseIndicatorList();

	if (m_pgDataManager->isSetCoordSystem()) {
		this->updateLandmarkAndReference();
		this->updateMeasureAndAnalysis(m_pSelectAnalysis->currentText());
	}
}
void CW3CephIndicatorBar::slotUpdateLandmarkPositions() {
	if (m_pgDataManager->isSetCoordSystem()) {
		this->updateLandmarkAndReference();
		this->updateMeasureAndAnalysis(m_pSelectAnalysis->currentText());
	}
}
