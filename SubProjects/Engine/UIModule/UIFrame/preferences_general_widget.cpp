#include "preferences_general_widget.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QToolButton>

#include <QApplication>
#include <QFileDialog>


#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Define.h"

namespace
{
	const QString kIPAdress = "127.0.0.1";
	const QString kCapturePath = "./screenshot/";
	const QString kStlExportPath = "./stl/";
}

PreferencesGeneralWidget::PreferencesGeneralWidget(QWidget *parent /*= nullptr*/)
	:BaseWidget(parent)
{
	contents_layout()->addLayout(CreateGeneralLayout());
}

PreferencesGeneralWidget::~PreferencesGeneralWidget()
{

}

void PreferencesGeneralWidget::SetGlobalPreferences()
{
	GlobalPreferences::Preferences *preferences = &GlobalPreferences::GetInstance()->preferences_;

#ifndef WILL3D_VIEWER
	preferences->general.network_setting.use_willmaster = network_setting_use_willmaster_check_->isChecked();
	preferences->general.network_setting.connection_type = network_setting_network_radio_->isChecked() ? GlobalPreferences::ConnectionType::Network : GlobalPreferences::ConnectionType::Local;
	preferences->general.network_setting.ip_address = network_setting_ip_address_input_->text();

#ifndef WILL3D_EUROPE
	QStringList favorite_open_paths;
	for (int i = 0; i < files_favorite_open_paths_list_->count(); ++i)
	{
		favorite_open_paths.push_back(files_favorite_open_paths_list_->item(i)->text());
	}
	preferences->general.files.favorite_open_paths = favorite_open_paths;
#endif // !WILL3D_EUROPE
#endif // !WILL3D_VIEWER
	
#ifndef WILL3D_EUROPE
	preferences->general.files.capture_path = files_capture_path_input_->text();
	preferences->general.files.stl_export_path = files_stl_export_path_input_->text();

	preferences->general.interfaces.language = static_cast<LanguageID>(interface_language_combo_->currentIndex());
	preferences->general.interfaces.gui_size = static_cast<GlobalPreferences::Size>(interface_gui_size_combo_->currentIndex());
	preferences->general.interfaces.font_size = static_cast<GlobalPreferences::Size>(interface_font_size_combo_->currentIndex());

	preferences->general.display.show_slice_numbers = display_show_slice_numbers_check_->isChecked();
	preferences->general.display.show_rulers = display_show_rulers_check_->isChecked();
	preferences->general.display.grid_spacing = display_grid_spacing_first_radio_->isChecked() ? 0 : display_grid_spacing_second_radio_->isChecked() ? 1 : 2;
#endif // !WILL3D_EUROPE
}

#ifndef WILL3D_VIEWER
void PreferencesGeneralWidget::slotSetEnableNetwork(int radio_id)
{
	bool is_enable = radio_id == static_cast<int>(GlobalPreferences::ConnectionType::Network) ? true : false;
	network_setting_ip_address_input_->setEnabled(is_enable);
}

void PreferencesGeneralWidget::slotAddNewPath()
{
	QString path = GetFolderPath();
	if (!path.isEmpty())
	{
		files_favorite_open_paths_list_->addItem(path);
	}
}

void PreferencesGeneralWidget::slotDeleteFavoriteOpenPaths()
{
	QList<QListWidgetItem*> selected_items = files_favorite_open_paths_list_->selectedItems();
	if (selected_items.size() < 1)
	{
		return;
	}

	for (int i = 0; i < selected_items.size(); ++i)
	{
		QListWidgetItem* item = selected_items.at(i);
		files_favorite_open_paths_list_->removeItemWidget(item);
		SAFE_DELETE_OBJECT(item);
	}
}
#endif

void PreferencesGeneralWidget::Reset()
{
#ifndef WILL3D_VIEWER
	//network
	network_setting_use_willmaster_check_->setChecked(false);
	network_setting_local_radio_->setChecked(true);
	network_setting_ip_address_input_->setText(kIPAdress);

	//file
	files_favorite_open_paths_list_->clear();
#endif
	files_capture_path_input_->setText(kCapturePath);
	files_stl_export_path_input_->setText(kStlExportPath);

	interface_language_combo_->setCurrentIndex(0);
	interface_gui_size_combo_->setCurrentIndex(0);
	interface_font_size_combo_->setCurrentIndex(0);

	display_show_slice_numbers_check_->setChecked(true);
	display_show_rulers_check_->setChecked(true);

	display_grid_spacing_third_radio_->setChecked(true);
}

QVBoxLayout* PreferencesGeneralWidget::CreateGeneralLayout()
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	layout->setSpacing(kSpacing10);
	layout->setContentsMargins(kContentsMargins);

#ifndef WILL3D_VIEWER
	layout->addWidget(CreateHorizontalLine());
	layout->addLayout(CreateNetworkSettingLayout());
#endif

#ifndef WILL3D_EUROPE
	layout->addWidget(CreateHorizontalLine());
	layout->addLayout(CreateFilesLayout());
	layout->addWidget(CreateHorizontalLine());
	layout->addLayout(CreateInterfaceLayout());
	layout->addWidget(CreateHorizontalLine());
	layout->addLayout(CreateDisplayLayout());
#endif // !WILL3D_EUROPE
	
	GetGeneralValues();

	return layout;
}

#ifndef WILL3D_VIEWER
QVBoxLayout* PreferencesGeneralWidget::CreateNetworkSettingLayout()
{
	QVBoxLayout* network_setting_layout = new QVBoxLayout();
	{
		QButtonGroup* radio_group = new QButtonGroup(this);

		QLabel* network_setting_title = CreateLabel(lang::LanguagePack::txt_network_setting(), QSizePolicy::Expanding, QSizePolicy::Fixed);
		QVBoxLayout* network_setting_contents_layout = new QVBoxLayout();
		{
			QHBoxLayout* network_setting_network_radio_layout = new QHBoxLayout();
			{
				network_setting_local_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_local());
				network_setting_network_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_network());

				radio_group->addButton(network_setting_local_radio_, static_cast<int>(GlobalPreferences::ConnectionType::Local));
				radio_group->addButton(network_setting_network_radio_, static_cast<int>(GlobalPreferences::ConnectionType::Network));

				network_setting_network_radio_layout->setSpacing(kSpacing5);
				network_setting_network_radio_layout->setContentsMargins(kStepMargins);

				network_setting_network_radio_layout->addWidget(network_setting_local_radio_);
				network_setting_network_radio_layout->addWidget(network_setting_network_radio_);
			}

			QHBoxLayout* network_setting_ip_address_layout = new QHBoxLayout();
			{
				QLabel* network_setting_ip_address_label = CreateLabel(lang::LanguagePack::txt_ip_address() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				network_setting_ip_address_input_ = CreateLineEdit();

				network_setting_ip_address_layout->setSpacing(kSpacing5);
				network_setting_ip_address_layout->setContentsMargins(kStepMargins);
				network_setting_ip_address_layout->addWidget(network_setting_ip_address_label);
				network_setting_ip_address_layout->addWidget(network_setting_ip_address_input_);
			}

			network_setting_use_willmaster_check_ = new QCheckBox(this);
			network_setting_use_willmaster_check_->setText("Will-Master");

			network_setting_contents_layout->setContentsMargins(kStepMargins);
			network_setting_contents_layout->addWidget(network_setting_use_willmaster_check_);
			network_setting_contents_layout->addLayout(network_setting_network_radio_layout);
			network_setting_contents_layout->addLayout(network_setting_ip_address_layout);
		}	

		network_setting_layout->addWidget(network_setting_title);
		network_setting_layout->addLayout(network_setting_contents_layout);

		connect(radio_group, SIGNAL(buttonClicked(int)), this, SLOT(slotSetEnableNetwork(int)));
		connect(network_setting_use_willmaster_check_, &QCheckBox::stateChanged, [=](int state)
		{
			bool is_ok = state == Qt::CheckState::Checked ? true : false;
			network_setting_local_radio_->setEnabled(is_ok);
			network_setting_network_radio_->setEnabled(is_ok);
			network_setting_ip_address_input_->setEnabled(is_ok && radio_group->checkedId() == static_cast<int>(GlobalPreferences::ConnectionType::Network));
		});
	}

	return network_setting_layout;
}
#endif

QVBoxLayout* PreferencesGeneralWidget::CreateFilesLayout()
{
	QVBoxLayout* files_layout = new QVBoxLayout();
	{
		QLabel* files_title = CreateLabel(lang::LanguagePack::txt_files(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		QVBoxLayout* files_contents_layout = new QVBoxLayout();
		{
			files_contents_layout->setContentsMargins(kStepMargins);

#ifndef WILL3D_VIEWER
			QHBoxLayout* files_favorite_open_paths_layout = new QHBoxLayout();
			{
				QLabel* favorite_open_paths_label = CreateLabel(lang::LanguagePack::txt_favorite_open_paths() + " :");

				files_favorite_open_paths_list_ = new QListWidget(this);
				files_favorite_open_paths_list_->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogListViewStyleSheet());
				files_favorite_open_paths_list_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
				files_favorite_open_paths_list_->setFixedHeight(60);

				QVBoxLayout* button_layout = new QVBoxLayout();
				{
					QToolButton* add_new_button = CreateTextToolButton(lang::LanguagePack::txt_add_new());
					QToolButton* delete_button = CreateTextToolButton(lang::LanguagePack::txt_delete());
					QToolButton* delete_all_button = CreateTextToolButton(lang::LanguagePack::txt_delete_all());

					button_layout->setAlignment(Qt::AlignTop);
					button_layout->addWidget(add_new_button);
					button_layout->addWidget(delete_button);
					button_layout->addWidget(delete_all_button);

					connect(add_new_button, &QToolButton::clicked, this, &PreferencesGeneralWidget::slotAddNewPath);
					connect(delete_button, &QToolButton::clicked, this, &PreferencesGeneralWidget::slotDeleteFavoriteOpenPaths);
					connect(delete_all_button, &QToolButton::clicked, [=]() { files_favorite_open_paths_list_->clear(); });
				}

				files_favorite_open_paths_layout->setAlignment(Qt::AlignTop);
				files_favorite_open_paths_layout->addWidget(favorite_open_paths_label, 2);
				files_favorite_open_paths_layout->addWidget(files_favorite_open_paths_list_, 3);
				files_favorite_open_paths_layout->addLayout(button_layout, 1);
			}

			files_contents_layout->addLayout(files_favorite_open_paths_layout);
#endif
			files_contents_layout->addLayout(CreateFilesPathLayout(lang::LanguagePack::txt_capture_path() + " :", &files_capture_path_input_));
			files_contents_layout->addLayout(CreateFilesPathLayout(lang::LanguagePack::txt_stl_export_path() + " :", &files_stl_export_path_input_));
		}

		files_layout->addWidget(files_title);
		files_layout->addLayout(files_contents_layout);
	}
	return files_layout;
}

QVBoxLayout* PreferencesGeneralWidget::CreateInterfaceLayout()
{
	QVBoxLayout* interface_layout = new QVBoxLayout();
	{
		QLabel* interface_title = CreateLabel(lang::LanguagePack::txt_interface(), QSizePolicy::Expanding, QSizePolicy::Fixed);
		QVBoxLayout* interface_contents_layout = new QVBoxLayout();
		{
			interface_contents_layout->setContentsMargins(kStepMargins);

			GlobalPreferences::Interface *interfaces = &GlobalPreferences::GetInstance()->preferences_.general.interfaces;
			interface_contents_layout->addLayout(CreateInterfaceComboBoxLayout(lang::LanguagePack::txt_language() + " :", &interface_language_combo_, interfaces->language_presets));
			interface_contents_layout->addLayout(CreateInterfaceComboBoxLayout(lang::LanguagePack::txt_gui_size() + " :", &interface_gui_size_combo_, interfaces->gui_string_list));
			interface_contents_layout->addLayout(CreateInterfaceComboBoxLayout(lang::LanguagePack::txt_font_size() + " :", &interface_font_size_combo_, interfaces->font_size_string_list));
		}

		interface_layout->addWidget(interface_title);
		interface_layout->addLayout(interface_contents_layout);
	}

	return interface_layout;
}

QVBoxLayout* PreferencesGeneralWidget::CreateDisplayLayout()
{
	QVBoxLayout* display_layout = new QVBoxLayout();
	{
		QLabel* display_title = CreateLabel(lang::LanguagePack::txt_display(), QSizePolicy::Expanding, QSizePolicy::Fixed);
		QVBoxLayout* display_contents_layout = new QVBoxLayout();
		{
			display_contents_layout->setContentsMargins(kStepMargins);

			QHBoxLayout* display_grid_spacing_radio_layout = new QHBoxLayout();
			{
				QLabel* display_grid_spacing_label = CreateLabel(lang::LanguagePack::txt_grid_spacing() + " :");

				QButtonGroup* grid_spacing_radio_group = new QButtonGroup(this);

				display_grid_spacing_first_radio_ = CreateTextRadioBttton("5mm");
				display_grid_spacing_second_radio_ = CreateTextRadioBttton("10mm");
				display_grid_spacing_third_radio_ = CreateTextRadioBttton("20mm");

				grid_spacing_radio_group->addButton(display_grid_spacing_first_radio_, 0);
				grid_spacing_radio_group->addButton(display_grid_spacing_second_radio_, 1);
				grid_spacing_radio_group->addButton(display_grid_spacing_third_radio_, 2);

				display_grid_spacing_radio_layout->setSpacing(kSpacing10);
				display_grid_spacing_radio_layout->addWidget(display_grid_spacing_label);
				display_grid_spacing_radio_layout->addWidget(display_grid_spacing_first_radio_);
				display_grid_spacing_radio_layout->addWidget(display_grid_spacing_second_radio_);
				display_grid_spacing_radio_layout->addWidget(display_grid_spacing_third_radio_);
			}

			display_show_slice_numbers_check_ = new QCheckBox(lang::LanguagePack::txt_show_slice_numbers(), this);
			display_show_rulers_check_ = new QCheckBox(lang::LanguagePack::txt_show_rulers(), this);

			display_contents_layout->addWidget(display_show_slice_numbers_check_);
			display_contents_layout->addWidget(display_show_rulers_check_);
			display_contents_layout->addLayout(display_grid_spacing_radio_layout);
		}

		display_layout->addWidget(display_title);
		display_layout->addLayout(display_contents_layout);
	}
	
	return display_layout;
}

QHBoxLayout* PreferencesGeneralWidget::CreateFilesPathLayout(const QString& text, QLineEdit** input)
{
	QHBoxLayout* path_layout = new QHBoxLayout();
	{
		if (*input == nullptr)
		{
			*input = CreateLineEdit();
		}

		QLabel* label = CreateLabel(text);
		QToolButton* button = CreateTextToolButton(lang::LanguagePack::txt_change());

		path_layout->addWidget(label, 2);
		path_layout->addWidget(*input, 3);
		path_layout->addWidget(button, 1);

		connect(button, &QToolButton::clicked, [=]()
		{
			QString path = GetFolderPath();
			if (!path.isEmpty())
			{
				(*input)->setText(path);
			}
		});
	}
	return path_layout;
}

QHBoxLayout* PreferencesGeneralWidget::CreateInterfaceComboBoxLayout(const QString& text, QComboBox** input, const QStringList& items)
{
	QHBoxLayout* layout = new QHBoxLayout();
	{
		QLabel* label = CreateLabel(text);
		QLabel* empty_space = CreateLabel("", QSizePolicy::Fixed, QSizePolicy::Preferred);
		empty_space->setFixedWidth(CW3Theme::getInstance()->size_button().width());

		if (*input == nullptr)
		{
			*input = CreateComboBox(items, false);
		}

		layout->addWidget(label, 2);
		layout->addWidget(*input, 3);
		layout->addWidget(empty_space, 1);
	}
	return layout;
}

void PreferencesGeneralWidget::GetGeneralValues()
{
	GlobalPreferences::Preferences *preferences = &GlobalPreferences::GetInstance()->preferences_;

#ifndef WILL3D_VIEWER
	bool use_willmaster = preferences->general.network_setting.use_willmaster;
	network_setting_use_willmaster_check_->setChecked(use_willmaster);

	GlobalPreferences::ConnectionType connection_type = preferences->general.network_setting.connection_type;
	network_setting_local_radio_->setChecked(connection_type == GlobalPreferences::ConnectionType::Local);
	network_setting_network_radio_->setChecked(connection_type == GlobalPreferences::ConnectionType::Network);
	network_setting_local_radio_->setEnabled(use_willmaster);
	network_setting_network_radio_->setEnabled(use_willmaster);
	network_setting_ip_address_input_->setText(preferences->general.network_setting.ip_address);
	network_setting_ip_address_input_->setEnabled(use_willmaster && network_setting_network_radio_->isChecked());

#ifndef WILL3D_EUROPE
	QStringList favorite_open_paths = preferences->general.files.favorite_open_paths;
	for (int i = 0; i < favorite_open_paths.size(); ++i)
	{
		if (!favorite_open_paths.at(i).isEmpty())
		{
			files_favorite_open_paths_list_->addItem(favorite_open_paths.at(i));
		}
	}
#endif // !WILL3D_EUROPE
#endif // !WILL3D_VIEWER

#ifndef WILL3D_EUROPE
	files_capture_path_input_->setText(preferences->general.files.capture_path);
	files_stl_export_path_input_->setText(preferences->general.files.stl_export_path);

	int language = static_cast<int>(preferences->general.interfaces.language);
	interface_language_combo_->setCurrentIndex((interface_language_combo_->count() > language) ? language : 0);

	int gui_size = static_cast<int>(preferences->general.interfaces.gui_size);
	interface_gui_size_combo_->setCurrentIndex((interface_gui_size_combo_->count() > gui_size) ? gui_size : 0);

	int font_size = static_cast<int>(preferences->general.interfaces.font_size);
	interface_font_size_combo_->setCurrentIndex((interface_font_size_combo_->count() > font_size) ? font_size : 0);

	display_show_slice_numbers_check_->setChecked(preferences->general.display.show_slice_numbers);
	display_show_rulers_check_->setChecked(preferences->general.display.show_rulers);
	display_grid_spacing_first_radio_->setChecked(preferences->general.display.grid_spacing == 0);
	display_grid_spacing_second_radio_->setChecked(preferences->general.display.grid_spacing == 1);
	display_grid_spacing_third_radio_->setChecked(preferences->general.display.grid_spacing == 2);
#endif // !WILL3D_EUROPE
}

QString PreferencesGeneralWidget::GetFolderPath()
{
	QString path;
#if defined(__APPLE__)
	path = QFileDialog::getExistingDirectory(this,
		"Select Folder"
		"/home",
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
#else
	path = QFileDialog::getExistingDirectory(this, "Select Folder", QApplication::applicationDirPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
#endif

	return path;
}
