#include "file_system_tree_widget.h"

#include <QDebug>
#include <QDir>
#include <QFileIconProvider>
#include <QDateTime>
#include <QHeaderView>
#include <qstorageinfo.h>
#include <QFileSystemModel>
#include <QStandardPaths>
#include <QCoreApplication>
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Theme.h"

#define WidonwHeaderOn 0
#if WidonwHeaderOn
#if defined(_WIN32)
#include <Windows.h>
#include <ShlObj.h>
#endif
#endif

FileSystemTreeWidget::FileSystemTreeWidget(const QString& root, QWidget* parent)
	: QTreeWidget(parent)
{
	setStyleSheet(CW3Theme::getInstance()->FileSystemTreeWidgetStyleSheet());

	setExpandsOnDoubleClick(false);
	setHeaderHidden(true);
	setColumnCount(3);
	hideColumn(2);
	QHeaderView* header = this->header();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
	header->setStretchLastSection(true);

	connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(slotExpanded(QTreeWidgetItem*)));

  QStringList arg_list = QCoreApplication::instance()->arguments();

  if(arg_list.count() <2)
	  InitTree(root);
}

FileSystemTreeWidget::~FileSystemTreeWidget()
{

}

void FileSystemTreeWidget::SelectPath(QString& path)
{
	QStringList path_part_list = path.replace("\\", "/").split("/");

	QTreeWidgetItem* item = FindPath(path_part_list);
	if (item)
	{
		setCurrentItem(item);
		scrollToItem(item);

		emit itemClicked(item, 0);
	}
}

void FileSystemTreeWidget::AddFavorite(const QString& path)
{
	if (path.isEmpty())
	{
		return;
	}

	QFileInfo folder(path);
	QTreeWidgetItem* item_root = new QTreeWidgetItem(this);

	favorite_list_.append(item_root);

	InitItem(item_root, folder);
	SetChild(item_root, folder.absoluteFilePath());
}

void FileSystemTreeWidget::AddFavorites(const QStringList& paths)
{
	for (int i = 0; i < favorite_list_.size(); ++i)
	{
		QTreeWidgetItem* item = favorite_list_.at(i);
		SAFE_DELETE_OBJECT(item);
	}

	favorite_list_.clear();

	for (int i = 0; i < paths.size(); ++i)
	{
		if (!paths.at(i).isEmpty())
		{
			AddFavorite(paths.at(i));
		}
	}
}

// private
void FileSystemTreeWidget::InitTree(const QString& path)
{
#if WidonwHeaderOn
	TCHAR t_desktop_path_buf[MAX_PATH];
	TCHAR t_documents_path_buf[MAX_PATH];

	SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, t_desktop_path_buf);
	SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, t_documents_path_buf);

	char desktop_path_buf[MAX_PATH];
	char documents_path_buf[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, t_desktop_path_buf, MAX_PATH, desktop_path_buf, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, t_documents_path_buf, MAX_PATH, documents_path_buf, MAX_PATH, NULL, NULL);

	QString desktop_path(desktop_path_buf);
	QString documents_path(documents_path_buf);
#else
	QString desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString documents_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif	
	QFileInfo desktop_folder(desktop_path);
	QFileInfo documents_folder(documents_path);	
	QTreeWidgetItem* desktop_item_root = new QTreeWidgetItem(this);
	QTreeWidgetItem* documents_item_root = new QTreeWidgetItem(this);
	InitItem(desktop_item_root, desktop_folder);
	InitItem(documents_item_root, documents_folder);
	SetChild(desktop_item_root, desktop_folder.absoluteFilePath());
	SetChild(documents_item_root, documents_folder.absoluteFilePath());

	QFileInfoList entry_list;
	if (path.isEmpty())
	{
		entry_list = QDir::drives();
	}
	else
	{
		QDir dir(path);
		dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden | QDir::System);
		entry_list = dir.entryInfoList();
	}

	QFileSystemModel fsm;
	fsm.setRootPath(path);
	QObject::connect(&fsm, &QFileSystemModel::layoutChanged, [&]()
	{
		qDebug() << "layoutChanged :" << fsm.rowCount(fsm.index(path));
	});

	for (int i = 0; i < entry_list.size(); ++i)
	{
		QFileInfo folder = entry_list.at(i);

		QTreeWidgetItem* item_root = new QTreeWidgetItem(this);

		InitItem(item_root, folder);
		SetChild(item_root, folder.absoluteFilePath());
	}
}

void FileSystemTreeWidget::InitItem(QTreeWidgetItem* item, const QFileInfo& file_info)
{
	int year = file_info.lastModified().date().year();
	int month = file_info.lastModified().date().month();
	int day = file_info.lastModified().date().day();
	int hour = file_info.lastModified().time().hour();
	int minute = file_info.lastModified().time().minute();

	QString ampm = "AM";
	if (hour >= 12)
	{
		ampm = "PM";
	}

	if (hour > 12)
	{
		hour -= 12;
	}

	QString last_modified = QString("%1-%2-%3 %4 %5:%6").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0')).arg(ampm).arg(hour, 2, 10, QChar('0')).arg(minute, 2, 10, QChar('0'));

	item->setText(0, file_info.isRoot() ? file_info.path().split("/").at(0) : file_info.fileName());
	item->setText(1, last_modified);
	item->setText(2, file_info.absoluteFilePath());

	QFileIconProvider icon_provider;
#if 0
	if (icon_provider.type(file_info).compare("File Folder", Qt::CaseInsensitive) == 0)
	{
		item->setIcon(0, icon_provider.icon(QFileIconProvider::Folder));
	}
	else
	{
		item->setIcon(0, icon_provider.icon(file_info));
	}
#else
	item->setIcon(0, icon_provider.icon(file_info));
#endif
}

void FileSystemTreeWidget::SetChild(QTreeWidgetItem* parent, const QString& absolute_path)
{
	QStorageInfo dir_info(absolute_path);
	if (!dir_info.isReady())
	{
		return;
	}
	QDir dir(absolute_path);
#if 0
	bool is_dir_exist = dir.isReadable();
	auto cnt = dir.count();
#endif
	dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden | QDir::System);
	QFileInfoList entry_list = dir.entryInfoList();
	
	for (int i = 0; i < entry_list.size(); ++i)
	{
		QFileInfo folder = entry_list.at(i);
		QTreeWidgetItem* item_child = new QTreeWidgetItem();

		InitItem(item_child, folder);

		parent->addChild(item_child);
	}
}

QTreeWidgetItem* FileSystemTreeWidget::FindPath(QStringList& path, QTreeWidgetItem* parent)
{
	if (path.size() == 0)
	{
		return parent;
	}

	QTreeWidgetItem* item = nullptr;

	if (parent)
	{
		for (int i = 0; i < parent->childCount(); ++i)
		{
			item = parent->child(i);
			if (item->text(0).compare(path.at(0), Qt::CaseInsensitive) == 0)
			{
				item->setExpanded(true);
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < topLevelItemCount(); ++i)
		{
			item = topLevelItem(i);
			if (item->text(0).compare(path.at(0), Qt::CaseInsensitive) == 0)
			{
				item->setExpanded(true);
				break;
			}
		}
	}

	if (item)
	{
		path.removeFirst();
		return FindPath(path, item);
	}
	else
	{
		return nullptr;
	}
}

// slots
void FileSystemTreeWidget::slotExpanded(QTreeWidgetItem* item)
{
	int child_child_count = 0;
	for (int i = 0; i < item->childCount(); ++i)
	{
		child_child_count += item->child(i)->childCount();
	}

	if (child_child_count == 0)
	{
		for (int i = 0; i < item->childCount(); ++i)
		{
			SetChild(item->child(i), item->child(i)->text(2));
		}
	}
}
