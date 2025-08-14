#pragma once

/*=========================================================================

File:			class CW3ToolBox
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-04-05
Last modify:	2016-04-05

=========================================================================*/
#include <memory>
#include <QFrame>
#include "uitools_global.h"

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QLabel;
class QLayout;

class UITOOLS_EXPORT ToolBox : public QFrame {
	Q_OBJECT
public:
	ToolBox(QFrame* parent = 0);
	~ToolBox();

public:
	virtual void setVisible(bool visible) override;
	void setCaptionName(const QString& name, Qt::Alignment align = Qt::AlignHCenter);
	void setContentsMargins(const QMargins& margin);
	void addToolWidget(QWidget* widget);
	void addToolLayout(QVBoxLayout* layout);
	void addToolLayout(QHBoxLayout* layout);
	void addToolLayout(QGridLayout* layout);

	void setCaptionVisible(const bool& visible);
	QLayout* GetLayout();
	inline QWidget* tool_widget() { return tool_widget_; }

	void ClearTools();

protected:
	std::unique_ptr<QHBoxLayout> content_layout_;
	std::unique_ptr<QLabel>	caption_;
	QWidget* tool_widget_ = nullptr; // 외부에서 포인터만 세팅됨
};
