#pragma once

/*=========================================================================

File:			class CW3CephSurgeryBar
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-07-04
Last modify:	2016-07-04

=========================================================================*/
#if defined(__APPLE__)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#else
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#endif
#include <QFrame>

#include "../../Common/Common/W3Enum.h"

#include "uitools_global.h"
class QVBoxLayout;
class QDoubleSpinBox;
class QCheckBox;
class QToolButton;
#ifndef WILL3D_VIEWER
class ProjectIOCeph;
#endif

class UITOOLS_EXPORT CephSurgeryTool : public QFrame {
	Q_OBJECT

public:
	explicit CephSurgeryTool(QWidget* parent = 0);
	virtual ~CephSurgeryTool();

public:
	inline void setEnabled(bool isEnable) noexcept { is_enable_ = isEnable; }
	inline bool isEnabled() const noexcept { return is_enable_; }

#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOCeph& out);
	void importProject(ProjectIOCeph& in);
#endif

signals:
	void sigSurgeryModeEnable(const bool); // from task tool

	void sigSurgeryEnableOn(const surgery::CutTypeID& cut_id, const bool isEnable);
	void sigSurgeryAdjustOn(const surgery::CutTypeID& cut_id, const bool isEnable);
	void sigSurgeryMoveOn(const surgery::CutTypeID& cut_id, const bool isEnable);
	void sigSurgeryCutTranslate(const surgery::CutTypeID& cut_id,
								const glm::mat4& trans_mat);
	void sigSurgeryAxisTranslate(const surgery::CutTypeID& cut_id,
								 const glm::mat4& trans_mat);
	void sigSurgeryCutRotate(const surgery::CutTypeID& cut_id,
							 const glm::mat4& rot_mat);
	void sigSurgeryAxisRotate(const surgery::CutTypeID& cut_id,
							  const glm::mat4& rot_mat);

public slots:
	void slotRotateFromView(const surgery::CutTypeID& cut_id,
							const surgery::RotateID& rot_id, float degree);
	void slotTranslateFromView(const surgery::CutTypeID& cut_id,
							   const glm::vec3& trans);
	void slotChangeEnable();
	void slotSurgeryParamsClear(const surgery::CutTypeID& cut_id);
	void slotSurgeryParamsDisable();

private slots:
	void slotEnable(int);
	void slotAdjustOn();
	void slotMoveOn();
	void slotTranslate(double);
	void slotRotate(double);

private:
	void connections();
	void setContentLayout(QVBoxLayout* layout);
	void initControlItems();

	void SetTextMoveToolButton(const surgery::CutTypeID& type);
	void SetEnableCutItems(surgery::CutTypeID type, bool is_enable);

	surgery::CutTypeID SenderID(QObject* sender);

private:
	struct SurgeryUI {
		QCheckBox* enable;
		QToolButton* adjust;
		QToolButton* move;
		QDoubleSpinBox* trans[surgery::TranslateID::TRANS_END];
		QDoubleSpinBox* rotate[surgery::RotateID::ROT_END];
	};
	
	struct SurgeryParams {
		float trans[surgery::TranslateID::TRANS_END];
		float rotate[surgery::RotateID::ROT_END];
	};

private:
	SurgeryUI ui_[surgery::CutTypeID::CUT_TYPE_END];
	SurgeryParams prev_params_[surgery::CutTypeID::CUT_TYPE_END];

	bool is_adjust_[surgery::CutTypeID::CUT_TYPE_END] = { false, false, false };
	bool is_move_[surgery::CutTypeID::CUT_TYPE_END] = { false, false, false };

	QToolButton* m_btnClose;

	bool is_enable_ = false;
};
