#include "W3REPORTtab.h"

#include "../../Engine/Common/Common/W3Memory.h"
#include "../../Engine/Common/Common/W3Logger.h"
#include "../../Engine/Resource/Resource/W3Image3D.h"
#include "../../Engine/Resource/ResContainer/resource_container.h"
#include "../../Engine/UIModule/UIFrame/W3ReportWindow.h"

using std::runtime_error;
using std::cout;
using std::endl;

CW3REPORTtab::CW3REPORTtab() {}

CW3REPORTtab::~CW3REPORTtab(void) {
	SAFE_DELETE_OBJECT(m_pLayout);
	SAFE_DELETE_OBJECT(m_pReport);
}

void CW3REPORTtab::SetLayout() {
	m_pLayout = new QVBoxLayout();
	m_pLayout->setSpacing(0);
	m_pLayout->setContentsMargins(0, 0, 0, 0);
	m_pLayout->addWidget(m_pReport);  
	tab_layout_ = m_pLayout;
 
}

void CW3REPORTtab::Initialize() {
	if (BaseTab::initialized()) {
		common::Logger::instance()->Print(common::LogType::ERR, "already initialized.");
		assert(false);
	}

	m_pReport = new CW3ReportWindow();
	m_pReport->initReport(ResourceContainer::GetInstance()->GetMainVolume().getHeader());

	SetLayout();
  report_rect_ = m_pReport->rect();

	BaseTab::set_initialized(true);
}

void CW3REPORTtab::SetVisibleWindows(bool isVisible) {
	if (!initialized())
		return;

  if (isVisible)
  {
    m_pReport->resize(report_rect_.width(), report_rect_.height());
    m_pReport->raise();
  }
  else
  {
    m_pReport->resize(QSize(0, 0));
    m_pReport->lower();

  }
	//m_pReport->setVisible(isVisible);
}

void CW3REPORTtab::addThumbnail(const QString &path) {
	if (m_pReport)
		m_pReport->addThumbnail(path);
}
