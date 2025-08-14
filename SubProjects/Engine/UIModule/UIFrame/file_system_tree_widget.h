#pragma once

/*=========================================================================

File:			class DicomLoader
Language:		C++11
Library:		Qt 5.8
Author:			Jung Dae Gun
First date:		2018-06-14
Last modify:	2018-06-14

Copyright (c) 2018 All rights reserved by HDXWILL.

=========================================================================*/

#include <QTreeWidget>

#include "uiframe_global.h"

class QFileInfo;
class UIFRAME_EXPORT FileSystemTreeWidget : public QTreeWidget 
{
	Q_OBJECT
public:
	FileSystemTreeWidget(const QString& root, QWidget* parent = nullptr);
	~FileSystemTreeWidget();

	void SelectPath(QString& path);
	void AddFavorite(const QString& path);
	void AddFavorites(const QStringList& paths);

private:
	void InitTree(const QString& path);
	void InitItem(QTreeWidgetItem* item, const QFileInfo& file_info);
	void SetChild(QTreeWidgetItem* parent, const QString& absolute_path);
	QTreeWidgetItem* FindPath(QStringList& path, QTreeWidgetItem* parent = nullptr);

private slots:
	void slotExpanded(QTreeWidgetItem* item);

private:
	QList<QTreeWidgetItem*> favorite_list_;
};

