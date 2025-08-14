#pragma once
/*=========================================================================
File:			class GlobalPreferences
Language:		C++11
Library:        Qt 5.9.9
Author:			Jung Dae Gun
First date:		2018-04-18
Last modify:	2021-08-31
=========================================================================*/
#include <QObject>
#include <QColor>
#include <QMutex>
#include <QSettings>
#include <QSize>

#include "language_pack.h"
#include "common_global.h"

class COMMON_EXPORT GlobalPreferences
{
public:
	enum class ConnectionType { Local, Network };
	enum class Size { Small, Medium, Large };
	enum class Quality2 { High, Low };
	enum class Quality3 { High, Medium, Low };
	enum class Direction { Normal, Inverse };
	enum class MeshRenderingType { Volume, Wire };
	enum class Reorientation { Auto, Manual };

	struct PreferencesCommon
	{
		bool reset_button_enable = false;
	};

	struct Tab
	{
		bool report_tab_hide = false;
	};

	struct Database
	{
		int version = 3;
		int port = 3307;
	};

	struct NetworkSetting
	{
		bool use_willmaster = false;
		ConnectionType connection_type = ConnectionType::Local;
		QString ip_address = "127.0.0.1";
	};

	struct Files
	{
		QStringList favorite_open_paths;
		QString capture_path = "./screenshot/";
		bool capture_path_with_patient_folder = true;
		QString stl_export_path = "./stl/";
	};

	struct Interface
	{
		const QStringList language_presets{ "English", "Chinese", "Chinese(Taiwan)", "Rusia", "Japanese", "Deutsch","Polish"};
		LanguageID language = LanguageID::EN;
		QStringList gui_string_list;
		Size gui_size = Size::Small;
		QStringList font_size_string_list;
		Size font_size = Size::Small;
		bool is_maximize = true;
		int maximize_type = 1;
	};

	struct Display
	{
		bool show_slice_numbers = true;
		bool show_rulers = true;
		const QList<int> grid_spacing_preset{ 5, 10, 20 };
		int grid_spacing = 0;
	};

	struct General
	{
		Database database;
		NetworkSetting network_setting;
		Files files;
		Interface interfaces;
		Display display;
	};

	struct Measure
	{
		QColor line_color = QColor(0, 255, 0);
		QColor text_color = QColor(255, 255, 0);
		QStringList text_size_string_list;
		Size text_size = Size::Small;
		float free_draw_line_width = 2.0f;
		bool tape_line_multi_label = false;
	};

	struct Nerve
	{
		double default_diameter = 2.0f;
		QColor default_color = QColor(255, 0, 0);
	};

	struct Implant
	{
		MeshRenderingType rendering_type = MeshRenderingType::Volume;
		QColor default_color_volume = QColor(64, 177, 255);
		QColor default_color_wire = QColor(64, 177, 255);
		QColor selected_color_volume = QColor(33, 255, 255);
		QColor selected_color_wire = QColor(33, 255, 255);
		QColor collided_color_volume = QColor(255, 0, 0);
		QColor collided_color_wire = QColor(255, 0, 0);
		double alpha = 0.4;
		double collision_margin = 5.0f;
		bool collision_margin_visible_on_2d_views = false;
		bool collision_margin_visible_on_3d_views = true;
		bool always_show_implant_id = true;
	};

	struct Objects
	{
		Measure measure;
		Nerve nerve;
		Implant implant;
	};

	struct MPR
	{
		bool hide_mpr_views_on_maximized_vr_layout = false;
		Reorientation reorientation = Reorientation::Auto;
		bool draw_arch = false;
		Direction sagittal_direction = Direction::Normal;
		double default_interval = 0.f; //0일 경우 pixel_spacing을 기본 값으로.
	};

	struct PanoramaView
	{
		bool move_axial_line_to_view_center = false;
	};

	struct CrossSectionVIew
	{
		double thickness_increments = 1.0f;
		double interval_increments = 0.5f;
		double default_interval = 1.0f;
		bool flip_slices_across_the_arch_centerline = true;
		bool slide_as_set = false;
		Direction direction = Direction::Normal;
		Direction mouse_wheel_direction = Direction::Normal;
		bool symmetry_by_implant = false;
		bool synchronize_angle_with_implant = false;
		bool shifted_by_interval = false;
		QSize default_layout = QSize(4, 1);
		bool move_axial_line_to_view_center = false;
	};

	struct PanoramaAxialView
	{
		double panorama_default_thickness = 10.0f;
		double panorama_thickness_increments = 1.0f;
		double panorama_default_range = 30.0f;
	};

	struct ImplantView
	{
		int translation_increments = 1;
		int rotation_increments = 1;
		int implant_handle_size_index = 3;
		int implant_handle_width_index = 3;
		bool cross_section_view_enable = false;
	};

	struct TMJ
	{
		QSize default_lateral_layout = QSize(3, 2);
	};

	struct VolumeRendering
	{
		QStringList quality_string_list;
		Quality2 quality = Quality2::High;
	};

	struct Windowing
	{
		bool use_fixed_value = false;
		int fixed_level = 1000;
		int fixed_width = 4000;
	};

	struct Advanced
	{
		MPR mpr;
		PanoramaView panorama_view;
		CrossSectionVIew cross_section_view;
		PanoramaAxialView panorama_axial_view;
		ImplantView implant_view;
		TMJ tmj;
		VolumeRendering volume_rendering;
		Windowing windowing;
	};

	struct Capture
	{
		bool include_dicom_info = false;
		QString image_format = "bmp";
		bool continuous = false;
	};

	struct STLExport
	{
		Quality3 quality = Quality3::High;
		bool smooth_on = true;
	};

	struct Login
	{
		int auto_logout_time = 30; //min
		bool save = false;
		bool auto_login = false;
		QString username;
		//QString password;
	};

	struct PACSDefaultSetting
	{
		//MPR
		bool mpr_view_list_swap = false;
		//MPR 2D
		bool mpr_is_2d_vertical = false;
		int mpr_2d_angle = 1;
		int mpr_interval = 1;
		int mpr_thickness = 1;
		int mpr_filter_level = 0;
		//MPR 3D rotation
		bool mpr_is_dir_posterior = false;
		bool mpr_is_3d_vertical = false;
		int mpr_3d_angle = 1;

		//Pano
	};

	struct PACS
	{
		bool pacs_on = false;
	};

	struct PACSServerInfo
	{
		PACSServerInfo() {}
		PACSServerInfo(const QString& _nickname, const QString& _ae_title, const QString& _ip_address, const QString& _port)
			: nickname(_nickname), ae_title(_ae_title), ip_address(_ip_address), port(_port) {}

		QString nickname;
		QString ae_title;
		QString ip_address;
		QString port;
	};

	struct PACSServerList
	{
		std::vector<PACSServerInfo> server_list;
		int select_num = -1;
	};

	struct Preferences
	{
		PreferencesCommon preferences_common;
		Tab tab;
		General general;
		Objects objects;
		Advanced advanced;
		Capture capture;
		STLExport stl_export;
		Login login;
		PACS pacs;
		PACSServerList pacs_server_list;
		PACSDefaultSetting pacs_default_setting;
		bool europe_window_btn_enable_ = false;
	};

	static GlobalPreferences* GetInstance()
	{
		static QMutex mutex;
		mutex.lock();
		if (!instance_)
		{
			instance_ = new GlobalPreferences();
			atexit(Destroy);
		}
		mutex.unlock();
		return instance_;
	}

	void Restore();

	void Load();
	void Save();

	void SavePreferences(); // for only preferences dialog
	void SaveCapture();
	void SaveSTLExport();
	void SaveLogin();

	bool GetShiftedByInterval();
	bool GetSynchronizeAngleWithImplant();
	bool GetSymmetryByImplant();

	//PACS
	void SelectPACSServer(const int index);
	void AddPACSServer(const QString& nickname, const QString& ae_title, const QString& ip, const QString& port);
	void RemovePACSServer(const int index);
	void GetPACSServerNicknameList(QStringList& out);
	int GetPACSServerSelectIndex();
	QString GetPACSSerisNumber();
	void AddCountPACSSerisNumber();
	QStringList GetPACSServerInfo(const int index);

	inline const QString& ini_path() const { return ini_path_; }
	inline const char* ini_codec_name() const { return ini_codec_name_; }
	QVariant GetINIData(QString main, QString sub, QString default, QString path);

private:
	static void Destroy()
	{
		static QMutex mutex;
		mutex.lock();
		if (instance_)
		{
			delete instance_;
			instance_ = nullptr;
		}
		mutex.unlock();
	}

	void LoadGeneral();
	void LoadObjects();
	void LoadAdvanced();

	void loadPACSDefaultSetting();
	void LoadPACSServerList();

	void SaveGeneral();
	void SaveObject();
	void SaveAdvanced();
	void SavePACSDefaultSetting();

	GlobalPreferences();
	GlobalPreferences(const GlobalPreferences&) {}
	const GlobalPreferences& operator = (const GlobalPreferences&) = delete;
	~GlobalPreferences();

public:
	Preferences preferences_;

private:
	static GlobalPreferences* instance_;

	const QString ini_path_ = "Will3D.ini";
	const char* ini_codec_name_ = "UTF-8";
};
