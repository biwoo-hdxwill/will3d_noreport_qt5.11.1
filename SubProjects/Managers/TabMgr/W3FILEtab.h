#pragma once
/*=========================================================================

File:			class CW3FILEtab
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun
First date:		2016-06-07
Last modify:	2016-06-07

=========================================================================*/
#include <memory>

#include "../../Engine/Common/Common/W3Enum.h"
#include "../../Engine/Core/W3DicomIO/W3dicomio.h"
#include "../../Engine/UIModule/UIFrame/dicom_loader.h"
#include "base_tab.h"

class CW3ResourceContainer;
class CW3DicomIO;
class CW3Image3D;
#ifndef WILL3D_VIEWER
class ProjectIOFile;
#endif

class CW3FILEtab : public BaseTab {
	Q_OBJECT

public:
	enum CURRNET_SETTING {
		FIRST_VOLUME,
		SECOND_VOLUME,
	};

public:
	CW3FILEtab(CW3ResourceContainer *Rcontainer, QWidget* parent = 0);

	virtual ~CW3FILEtab(void);

	// serialization
#ifndef WILL3D_VIEWER
	bool ImportMainVolumeFromProject(ProjectIOFile& in);
	void exportProject(ProjectIOFile& out);
	void importProject(ProjectIOFile& in);
#endif

	virtual void SetVisibleWindows(bool isVisible) override;

#ifndef WILL3D_VIEWER
	void insertDBforProject(DicomLoader::PatientInfo *patient) {
		dicom_loader_->InsertDBforProject(patient);
	}

	inline void setEnableLoadSecondVolume(bool isSecond) {
		m_bLoadSecondVolume = isSecond;
		dicom_loader_->SetEnableLoadSecondVolume(isSecond);
	}
#endif

	inline QString getOpendVolPath() {
		return dicom_loader_->opend_volume_path();
	}

	bool loadDicom();
	void runLoadDicom();

	inline CURRNET_SETTING getCurrentSetting() const { return (m_bLoadSecondVolume) ? SECOND_VOLUME : FIRST_VOLUME; }
	inline bool isProject() const {
		return dicom_loader_->is_project();
	}
	inline CW3Image3D* getVolume() const { return volume_.get(); }
	inline CW3Image3D* getSecondVolume() const { return second_volume_.get(); }
	inline QString getProjectPath() {
		return dicom_loader_->project_path();
	}

	void readInputFile(const QString& readFilePath);	// call from willmaster

	inline bool isOnlyTRD() const {
		return dicom_loader_->open_only_trd();
	}

#ifdef WILL3D_VIEWER
	bool ImportDICOM();
#endif
	void exportDICOM(QString export_path, bool is_include_viewer, bool is_dicom_compressed);

	void ApplyPreferences();

signals:
	void sigLoadProject(QString);
	void sigLoadDicomFromLoader();
	void sigLoadSecondDicomFromLoader();
	void sigSetTRDFromExternalProgram(const QString, const bool);

private slots:
	void slotLoadDicomFromLoader();
	void slotAutoOpenDicomFile();

private:
	virtual void SetLayout() override;
	virtual void Initialize() override;
	void connections();

private:
	std::shared_ptr<CW3Image3D>		volume_ = nullptr;
	std::shared_ptr<CW3Image3D>		second_volume_ = nullptr;

	CW3DicomIO		m_dicomIO;
	DicomLoader *dicom_loader_ = nullptr;

	bool			m_bLoadSecondVolume = false;
	QString			m_errLoadDicom = QString();

	QVBoxLayout*	m_pLayout = nullptr;
	QTimer* auto_open_dicom_file_timer_ = nullptr;

	QRect file_tab_rect_;
};
