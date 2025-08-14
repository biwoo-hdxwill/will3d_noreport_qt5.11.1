#pragma once

/**=================================================================================================

Project:		UIFrame
File:			nerve_tool_box.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-03-02
Last modify: 	2018-03-02

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>
#include <QFrame>

class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class QCheckBox;
class QDoubleSpinBox;
class QToolButton;
//20250123 LIN 주석처리
//#ifndef WILL3D_VIEWER
class ProjectIOPanoEngine;
//#endif

class NerveToolRecord : public QFrame {
	Q_OBJECT
public:
	NerveToolRecord(int record_id);
	~NerveToolRecord();
	NerveToolRecord(const NerveToolRecord&) = delete;
	NerveToolRecord& operator=(const NerveToolRecord&) = delete;

signals:
	void sigChangedValues(int record_id);
	void sigHovered(int record_id, bool is_hovered);
	void sigDelete(int record_id);

public:
	bool IsNerveVisible() const;
	float GetDiameter() const;
	void SetCheckedVisible(const bool & is_checked);
	void SetColor(const QColor& color);
	void SetDiameter(float diameter);

	inline const QColor& curr_color() const { return curr_color_; }
private slots:
	void slotClickedColor();
	void slotClickedDelete();
	void slotChangedValues();

private:
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);

private:
	std::unique_ptr<QHBoxLayout> layout_main_;
	std::unique_ptr<QCheckBox> check_visible_;
	std::unique_ptr<QLabel> text_id_;
	std::unique_ptr<QDoubleSpinBox> spin_dia_;
	std::unique_ptr<QToolButton> btn_color_;
	std::unique_ptr<QToolButton> btn_delete_;
	QColor curr_color_;
	int id_;
};

class NerveToolBox : public QFrame {
	Q_OBJECT

public:
	explicit NerveToolBox(QWidget* parent = 0);
	~NerveToolBox();

	NerveToolBox(const NerveToolBox&) = delete;
	NerveToolBox& operator=(const NerveToolBox&) = delete;

signals:
	void sigChangedNerveValues(const bool& nerve_visible, const int& nerve_id,
							   const float& diameter, const QColor& color);
	void sigDeleteNerve(int nerve_id);
	void sigToggledDraw(bool is_activated);
	void sigHoveredNerveRecord(int nerve_id, bool is_hovered);

public:
//20250123 LIN importProject viewer에 적용
//#ifndef WILL3D_VIEWER
	void importProject(ProjectIOPanoEngine& in);
//#endif

	void CreateRecord(int id);
	void DeleteRecord(int id);
	void ClearRecords();
	void PressDrawButton(bool is_disconnect_signal);
	void ReleaseDrawButton();
	void ChangeVisibleCheck(bool is_visible);

	bool IsPressDrawButton() const;
	QVBoxLayout* GetLayout() const { return layout_main_.get(); }

	bool IsVisibleNerveExists() const;

private:
	QLabel* CreateFieldLabel(const QString& text);

	void InitializeFrameField();
	void InitializeRecordArea();
	void InitializeButtons();

private slots:
	void slotChangedRecordValues(int record_id);
	void slotDrawModifyOn(bool toggle);

private:
	std::unique_ptr<QVBoxLayout> layout_main_;

	std::unique_ptr<QFrame> frame_field_;
	std::unique_ptr<QHBoxLayout> layout_buttons_;
	std::unique_ptr<QVBoxLayout> layout_records_;
	std::map<int, std::unique_ptr<NerveToolRecord>> records_;

	std::unique_ptr<QToolButton> btn_draw_modify_;

	int id_;
	bool is_visible_check_ = true;
};
