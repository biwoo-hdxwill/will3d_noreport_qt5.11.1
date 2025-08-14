#include "global_preferences.h"

#include <QTextCodec>
#include <QFile>
#include <QDebug>

GlobalPreferences* GlobalPreferences::instance_ = nullptr;

namespace
{
	// Preperences
	const QString kPreperencesSectionName("PREFERENCES");
	const QString kKeyResetButtonEnable("/reset_button_enable");

	// Tab
	const QString kTabSectionName("TAB");
	const QString kKeyReportTabHide("/report_tab_hide");

	// General
	const QString kDatabaseSectionName("DATABASE");
	const QString kKeyVersion("/version");
	const QString kKeyPort("/port");

	const QString kNetworkSettingSectionName("NETWORK_SETTING");
	const QString kKeyUseWillmaster("/use_willmaster");
	const QString kKeyConnectionType("/connection_type");
	const QString kKeyIpAddress("/ip_address");

	const QString kFilesSectionName("FILES");
	const QString kKeyFavoriteOpenPaths("/favorite_open_paths");
	const QString kKeyCapturePath("/capture_path");
	const QString kKeyCapturePathWithPatientFolder("/capture_path_with_patient_folder");
	const QString kKeySTLExportPath("/stl_export_path");

	const QString kInterfaceSectionName("INTERFACE");
	const QString kKeyLanguage("/language");
	const QString kKeyGUISize("/gui_size");
	const QString kKeyFontSize("/font_size");
	const QString kKeyMaximized("/maximized");
	const QString kKeyMaximizeType("/maximize_type");

	const QString kDisplaySectionName("DISPLAY");
	const QString kKeyShowSliceNumbers("/show_slice_numbers");
	const QString kKeyShowRulers("/show_rulers");
	const QString kKeyGridSpacing("/grid_spacing");

	// Objects
	const QString kMeasureSectionName("MEASURE");
	const QString kKeyLineColor("/line_color");
	const QString kKeyTextColor("/text_color");
	const QString kKeyFreeDrawLineWidth("/free_draw_line_width");
	const QString kKeyTextSize("/text_size");
	const QString kKeyTapeLineMultiLabel("/tape_line_multi_label");

	const QString kNerveSectionName("NERVE");
	const QString kKeyDefaultDiameter("/default_diameter");
	const QString kKeyDefaultColor("/default_color");
	const QString kKeyShowNerve("/show_nerve");

	const QString kImplantSectionName("IMPLANT");
	const QString kKeyWire("/wire");
	const QString kKeyDefaultColorVolume("/default_color_volume");
	const QString kKeyDefaultColorWire("/default_color_wire");
	const QString kKeySelectedColorVolume("/selected_color_volume");
	const QString kKeySelectedColorWire("/selected_color_wire");
	const QString kKeyCollisionColorVolume("/collision_color_volume"); // 1.2.0.0
	const QString kKeyCollisionColorWire("/collision_color_wire"); // 1.2.0.0
	const QString kKeyCollidedColorVolume("/collided_color_volume"); // 1.3.0.0 rename : collided_color_volume -> collided_color_volume
	const QString kKeyCollidedColorWire("/collided_color_wire"); // 1.3.0.0 rename : collided_color_wire -> collided_color_wire
	const QString kKeyAlpha("/alpha");
	const QString kKeyCollisionMargin("/collision_margin");
	const QString kKeyCollisionMarginVisibleOn2DViews("/collision_margin_visible_on_2d_views");
	const QString kKeyCollisionMarginVisibleOn3DViews("/collision_margin_visible_on_3d_views");
	const QString kKeyAlwaysShowImplantId("/always_show_implant_id");

	// Advanced
	const QString kMPRSectionName("MPR");
	const QString kKeyHideMPRViewOnMaximizedVRLayout("/hide_mpr_views_on_maximized_vr_layout");
	const QString kKeyAutoOrientation("/auto_orientation");
	const QString kKeyDrawArch("/draw_arch");
	const QString kKeySagittalDirection("/sagittal_direction");

	const QString kPanoramaViewSectionName("PANORAMA_VIEW");
	const QString kKeyMoveAxialLineToViewCenter("/move_axial_line_to_view_center");

	const QString kCrossSectionViewSectionName("CROSS_SECTION_VIEW");
	const QString kKeyThicknessIncrements("/thickness_increments");
	const QString kKeyIntervalIncrements("/interval_increments");
	const QString kKeyDefaultInterval("/default_interval");
	const QString kKeyFlipSlicesAcrossTheArchCenterline("/flip_slices_across_the_arch_centerline");
	const QString kKeySlideAsSet("/slide_as_set");
	const QString kKeyDirection("/direction");
	const QString kKeyMouseWheelDirection("/mouse_wheel_direction");
	const QString kKeySymmetryByImplant("/symmetry_by_implant");
	const QString kKeySynchronizeAngleWithImplant("/synchronize_angle_with_implant");
	const QString kKeyShiftedByInterval("/shifted_by_interval");
	const QString kKeyDefaultLayout("/default_layout");

	const QString kPanoramaAxialViewSectionName("PANORAMA_AXIAL_VIEW");
	const QString kKeyPanoramaDefaultThickness("/panorama_default_thickness");
	const QString kKeyPanoramaThicknessIncrements("/panorama_thickness_increments");
	const QString kKeyPanoramaDefaultRange("/panorama_default_range");

	const QString kImplantViewSectionName("IMPLANT_VIEW");
	const QString kKeyImplantTranslationIncrements("/implant_translation_increments");
	const QString kKeyImplantRotationIncrements("/implant_rotation_increments");
	const QString kKeyImplantHandleSize("/implant_handle_size_index");
	const QString kKeyImplantHandleWidth("/implant_handle_width_index");
	const QString kKeyCrossSectionViewEnable("/cross_section_view_enable");

	const QString kTMJSectionName("TMJ");
	const QString kKeyDefaultLateralLayout("/default_lateral_layout");

	const QString kVolumeRenderingSectionName("VOLUME_RENDERING");
	const QString kKeyVolumeRenderingQuality("/quality");

	const QString kWindowingSectionName("Windowing");
	const QString kKeyUseFixedValue("/use_fixed_value");
	const QString kKeyFixedLevel("/fixed_level");
	const QString kKeyFixedWidth("/fixed_width");

	// Capture
	const QString kCaptrueSectionName("CAPTURE");
	const QString kKeyIncludeDicomInfo("/include_dicom_info");
	const QString kKeyImageFormat("/image_format");
	const QString kKeyContinuous("/continuous");

	// STLExport
	const QString kSTLExportSectionName("STL_EXPORT");
	const QString kKeySTLQuality("/quality");
	const QString kKeySmoothOn("/smooth_on");

	// Login
	const QString kLoginSectionName("LOGIN");
	const QString kKeyAutoLogoutTime("/auto_logout_time");
	const QString kKeySave("/save");
	const QString kKeyAutoLogin("/auto_login");
	const QString kKeyUsername("/username");
	//const QString kKeyPassword("/password");

	const int kMinAutoLogoutTime = 30;		//30분
	const int kMaxAutoLogoutTime = 21600;	//15일

	// Common
	const QString kCommonName("COMMON");
	const QString kEuropeWindowBtnEnable("/europe_window_btn_enable");
}

namespace pacs
{
	const QString kPACS = "PACS";
	const QString kPACSOn = "pacs_on";

	//PACS PREFERENCE
	const QString kPACSPreference = "PACS_PREFERENCE";
	const QString kMPRViewListSwap = "mpr_view_list_swap";
	const QString kMPRIs2DVertical = "mpr_is_2d_vertical";
	const QString kMPR2DAngle = "mpr_2d_angle";
	const QString kMPRInterval = "mpr_interval";
	const QString kMPRThickness = "mpr_thickness";
	const QString kMPRFilterLevel = "mpr_filter_level";
	const QString kMPRIs3DVertical = "mpr_is_3d_vertical";
	const QString kMPRIsDirPosterior = "mpr_is_dir_posterior";
	const QString kMPR3DAngle = "mpr_3d_angle";

	//PACS SERVER
	const QString kPACSServer = "PACS_SERVER";
	const QString kServerCnt = "server_cnt";
	const QString kSelectNum = "select_num";
	const QString kSeriesNum = "series_num";

	const QString kPACSServerINFO = "PACS_SERVER_INFO";
	const QString kNickname = "nickname";
	const QString kAETitle = "ae_title";
	const QString kIPAddress = "ip_address";
	const QString kPort = "port";

	const int kMaxCount = 99999;
}

GlobalPreferences::GlobalPreferences()
{
	preferences_.general.interfaces.gui_string_list = QStringList{ lang::LanguagePack::txt_small(), lang::LanguagePack::txt_medium(), lang::LanguagePack::txt_large() };
	preferences_.general.interfaces.font_size_string_list = QStringList{ lang::LanguagePack::txt_small(), lang::LanguagePack::txt_medium(), lang::LanguagePack::txt_large() };
	preferences_.objects.measure.text_size_string_list = QStringList{ lang::LanguagePack::txt_small(), lang::LanguagePack::txt_medium(), lang::LanguagePack::txt_large() };
	preferences_.advanced.volume_rendering.quality_string_list = QStringList{ lang::LanguagePack::txt_high(), lang::LanguagePack::txt_low() };

	Load();
	LoadPACSServerList();
}

GlobalPreferences::~GlobalPreferences()
{
	Save();
}

void GlobalPreferences::Restore()
{
	QString old_ini_path = "Will3D_old.ini";
	QFile old_ini_file(old_ini_path);
	if (!old_ini_file.exists())
	{
		return;
	}

	QSettings new_settings(ini_path_, QSettings::IniFormat);
	new_settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	QSettings old_settings(old_ini_path, QSettings::IniFormat);
	old_settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	QStringList old_keys = old_settings.allKeys();
	for (int i = 0; i < old_keys.size(); ++i)
	{
		const QString& key = old_keys.at(i);
		if (!new_settings.contains(key) ||
			key.compare("DATABASE/version", Qt::CaseInsensitive) == 0)
		{
			continue;
		}

		new_settings.setValue(key, old_settings.value(key, QVariant()));
	}

	old_ini_file.remove();
}

void GlobalPreferences::Load()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	// Common
	preferences_.preferences_common.reset_button_enable = settings.value(kPreperencesSectionName + kKeyResetButtonEnable, preferences_.preferences_common.reset_button_enable).toBool();
	preferences_.tab.report_tab_hide = settings.value(kTabSectionName + kKeyReportTabHide, preferences_.tab.report_tab_hide).toBool();
	preferences_.europe_window_btn_enable_ = settings.value(kCommonName + kEuropeWindowBtnEnable, preferences_.europe_window_btn_enable_).toBool();

	LoadGeneral();
	LoadObjects();
	LoadAdvanced();
	
	// Capture
	preferences_.capture.include_dicom_info =
		settings.value(kCaptrueSectionName + kKeyIncludeDicomInfo, preferences_.capture.include_dicom_info).toBool();
	preferences_.capture.image_format =
		settings.value(kCaptrueSectionName + kKeyImageFormat, preferences_.capture.image_format).toString();
	preferences_.capture.continuous =
		settings.value(kCaptrueSectionName + kKeyContinuous, preferences_.capture.continuous).toBool();
	// Capture

	// STLExport
	preferences_.stl_export.quality =
		static_cast<Quality3>(settings.value(kSTLExportSectionName + kKeySTLQuality, static_cast<int>(preferences_.stl_export.quality)).toInt());
	preferences_.stl_export.smooth_on =
		settings.value(kSTLExportSectionName + kKeySmoothOn, preferences_.stl_export.smooth_on).toBool();
	// STLExport

	// Login
	preferences_.login.auto_logout_time =
		settings.value(kLoginSectionName + kKeyAutoLogoutTime, preferences_.login.auto_logout_time).toInt();

	if (preferences_.login.auto_logout_time < kMinAutoLogoutTime)
	{
		preferences_.login.auto_logout_time = kMinAutoLogoutTime;
	}
	else if (preferences_.login.auto_logout_time >kMaxAutoLogoutTime)
	{
		preferences_.login.auto_logout_time = kMaxAutoLogoutTime;
	}

  preferences_.login.save = true;
		//settings.value(kLoginSectionName + kKeySave, preferences_.login.save).toBool();
  preferences_.login.auto_login = true;
		//settings.value(kLoginSectionName + kKeyAutoLogin, preferences_.login.auto_login).toBool();
	preferences_.login.username =
		settings.value(kLoginSectionName + kKeyUsername, preferences_.login.username).toString();
	/*preferences_.login.password =
		settings.value(kLoginSectionName + kKeyPassword, preferences_.login.password).toString();*/
		// Login

	loadPACSDefaultSetting();
}

void GlobalPreferences::Save() 
{
	SavePreferences();
	SaveCapture();
	SaveSTLExport();
}

void GlobalPreferences::SavePreferences() 
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));
		
	SaveGeneral();
	SaveObject();
	SaveAdvanced();
	SavePACSDefaultSetting();
}

void GlobalPreferences::SaveCapture() 
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.setValue(kCaptrueSectionName + kKeyIncludeDicomInfo, preferences_.capture.include_dicom_info);
	settings.setValue(kCaptrueSectionName + kKeyImageFormat, preferences_.capture.image_format);
#if 0 // 설정창에 없는 항목들은 저장하지 않음
	settings.setValue(kCaptrueSectionName + kKeyContinuous, preferences_.capture.continuous);
#endif
}

void GlobalPreferences::SaveSTLExport() 
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.setValue(kSTLExportSectionName + kKeySTLQuality, static_cast<int>(preferences_.stl_export.quality));
	settings.setValue(kSTLExportSectionName + kKeySmoothOn, preferences_.stl_export.smooth_on);
}

void GlobalPreferences::SaveLogin() 
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.setValue(kLoginSectionName + kKeySave, preferences_.login.save);
	settings.setValue(kLoginSectionName + kKeyAutoLogin, preferences_.login.auto_login);
	settings.setValue(kLoginSectionName + kKeyUsername, preferences_.login.username);
	//settings.setValue(kLoginSectionName + kKeyPassword, preferences_.login.password);
}

bool GlobalPreferences::GetShiftedByInterval()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	bool shifted_by_interval = preferences_.advanced.cross_section_view.shifted_by_interval;
	return settings.value(kCrossSectionViewSectionName + kKeyShiftedByInterval, shifted_by_interval).toBool();
}

bool GlobalPreferences::GetSynchronizeAngleWithImplant()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	bool synchronize_angle_with_implant = preferences_.advanced.cross_section_view.synchronize_angle_with_implant;
	return settings.value(kCrossSectionViewSectionName + kKeySynchronizeAngleWithImplant, synchronize_angle_with_implant).toBool();
}

bool GlobalPreferences::GetSymmetryByImplant()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	bool symmetry_by_implant = preferences_.advanced.cross_section_view.symmetry_by_implant;
	return settings.value(kCrossSectionViewSectionName + kKeySymmetryByImplant, symmetry_by_implant).toBool();
}

void GlobalPreferences::SelectPACSServer(const int index)
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.beginGroup(pacs::kPACSServer);
	settings.setValue(pacs::kSelectNum, index);
	settings.endGroup();

	preferences_.pacs_server_list.select_num = index;
}

void GlobalPreferences::AddPACSServer(const QString& nickname, const QString& ae_title, const QString& ip, const QString& port)
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	QString server_cnt = settings.value(pacs::kPACSServer + "/" + pacs::kServerCnt).toString();

	settings.beginGroup(pacs::kPACSServerINFO + "_" + server_cnt);
	settings.setValue(pacs::kNickname, nickname);
	settings.setValue(pacs::kAETitle, ae_title);
	settings.setValue(pacs::kIPAddress, ip);
	settings.setValue(pacs::kPort, port);
	settings.endGroup();

	settings.setValue(pacs::kPACSServer + "/" + pacs::kServerCnt, server_cnt.toInt() + 1);

	preferences_.pacs_server_list.server_list.emplace_back(PACSServerInfo(nickname, ae_title, ip, port));
}

void GlobalPreferences::RemovePACSServer(const int index)
{
	std::vector<PACSServerInfo>& list = preferences_.pacs_server_list.server_list;
	if (list.empty() || list.size() <= index)
	{
		return;
	}

	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.beginGroup(pacs::kPACSServer);
	int server_cnt = settings.value(pacs::kServerCnt).toInt();
	int select_num = settings.value(pacs::kSelectNum).toInt();

	settings.setValue(pacs::kServerCnt, server_cnt - 1);

	if (select_num == index)
	{
		settings.setValue(pacs::kSelectNum, -1);
		preferences_.pacs_server_list.select_num = -1;
	}
	else if (select_num > index)
	{
		settings.setValue(pacs::kSelectNum, select_num - 1);
		preferences_.pacs_server_list.select_num = select_num - 1;
	}
	settings.endGroup();

	for (int i = 0; i < server_cnt; ++i)
	{
		settings.remove(pacs::kPACSServerINFO + "_" + QString::number(i));
	}

	list.erase(list.begin() + index);

	int size = list.size();
	for (int i = 0; i < size; ++i)
	{
		settings.beginGroup(pacs::kPACSServerINFO + "_" + QString::number(i));
		settings.setValue(pacs::kNickname, list[i].nickname);
		settings.setValue(pacs::kAETitle, list[i].ae_title);
		settings.setValue(pacs::kIPAddress, list[i].ip_address);
		settings.setValue(pacs::kPort, list[i].port);
		settings.endGroup();
	}
}

void GlobalPreferences::GetPACSServerNicknameList(QStringList& out)
{
	std::vector<PACSServerInfo>& list = preferences_.pacs_server_list.server_list;
	if (list.empty())
	{
		return;
	}

	out.clear();
	int size = list.size();
	for (int i = 0; i < size; ++i)
	{
		out << list[i].nickname;
	}
}

int GlobalPreferences::GetPACSServerSelectIndex()
{
	return preferences_.pacs_server_list.select_num;
}

QString GlobalPreferences::GetPACSSerisNumber()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.beginGroup(pacs::kPACSServer);
	QString value = settings.value(pacs::kSeriesNum).toString();
	settings.endGroup();

	return value;
}

void GlobalPreferences::AddCountPACSSerisNumber()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.beginGroup(pacs::kPACSServer);
	int value = settings.value(pacs::kSeriesNum).toInt();
	if (value == pacs::kMaxCount)
	{
		value = 1;
	}
	else
	{
		value += 1;
	}
	settings.setValue(pacs::kSeriesNum, value);
	settings.endGroup();
}

QStringList GlobalPreferences::GetPACSServerInfo(const int index)
{
	QString nickname = preferences_.pacs_server_list.server_list[index].nickname;
	QString ae_title = preferences_.pacs_server_list.server_list[index].ae_title;
	QString ip_address = preferences_.pacs_server_list.server_list[index].ip_address;
	QString port = preferences_.pacs_server_list.server_list[index].port;

	QStringList info;
	info << nickname << ae_title << ip_address << port;

	return info;
}

void GlobalPreferences::LoadGeneral()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));
	
	preferences_.general.database.port = settings.value(kDatabaseSectionName + kKeyPort, preferences_.general.database.port).toInt();
	preferences_.general.database.version = settings.value(kDatabaseSectionName + kKeyVersion, preferences_.general.database.version).toInt();

	preferences_.general.network_setting.use_willmaster =
		settings.value(kNetworkSettingSectionName + kKeyUseWillmaster, preferences_.general.network_setting.use_willmaster).toBool();
	preferences_.general.network_setting.connection_type =
		static_cast<ConnectionType>(settings.value(kNetworkSettingSectionName + kKeyConnectionType, static_cast<int>(preferences_.general.network_setting.connection_type)).toInt());
	preferences_.general.network_setting.ip_address =
		settings.value(kNetworkSettingSectionName + kKeyIpAddress, preferences_.general.network_setting.ip_address).toString();

	preferences_.general.files.favorite_open_paths =
		settings.value(kFilesSectionName + kKeyFavoriteOpenPaths, preferences_.general.files.favorite_open_paths).toStringList();
	preferences_.general.files.capture_path =
		settings.value(kFilesSectionName + kKeyCapturePath, preferences_.general.files.capture_path).toString();
	preferences_.general.files.capture_path_with_patient_folder =
		settings.value(kFilesSectionName + kKeyCapturePathWithPatientFolder, preferences_.general.files.capture_path_with_patient_folder).toBool();
	preferences_.general.files.stl_export_path =
		settings.value(kFilesSectionName + kKeySTLExportPath, preferences_.general.files.stl_export_path).toString();

	preferences_.general.interfaces.language =
		static_cast<LanguageID>(settings.value(kInterfaceSectionName + kKeyLanguage, static_cast<int>(preferences_.general.interfaces.language)).toInt());
	preferences_.general.interfaces.gui_size =
		static_cast<Size>(settings.value(kInterfaceSectionName + kKeyGUISize, static_cast<int>(preferences_.general.interfaces.gui_size)).toInt());
	preferences_.general.interfaces.font_size =
		static_cast<Size>(settings.value(kInterfaceSectionName + kKeyFontSize, static_cast<int>(preferences_.general.interfaces.font_size)).toInt());
	preferences_.general.interfaces.is_maximize =
		settings.value(kInterfaceSectionName + kKeyMaximized, preferences_.general.interfaces.is_maximize).toBool();
	preferences_.general.interfaces.maximize_type =
		settings.value(kInterfaceSectionName + kKeyMaximizeType, preferences_.general.interfaces.maximize_type).toInt();

	preferences_.general.display.show_slice_numbers =
		settings.value(kDisplaySectionName + kKeyShowSliceNumbers, preferences_.general.display.show_slice_numbers).toBool();
	preferences_.general.display.show_rulers =
		settings.value(kDisplaySectionName + kKeyShowRulers, preferences_.general.display.show_rulers).toBool();
	preferences_.general.display.grid_spacing =
		settings.value(kDisplaySectionName + kKeyGridSpacing, preferences_.general.display.grid_spacing).toInt();
}

void GlobalPreferences::LoadObjects()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));
	
	QString line_color = settings.value(kMeasureSectionName + kKeyLineColor, preferences_.objects.measure.line_color.name()).toString();
	preferences_.objects.measure.line_color.setNamedColor(line_color);
	QString text_color = settings.value(kMeasureSectionName + kKeyTextColor, preferences_.objects.measure.text_color.name()).toString();
	preferences_.objects.measure.text_color.setNamedColor(text_color);
	preferences_.objects.measure.text_size =
		static_cast<Size>(settings.value(kMeasureSectionName + kKeyTextSize, static_cast<int>(preferences_.objects.measure.text_size)).toInt());
	preferences_.objects.measure.free_draw_line_width = settings.value(kMeasureSectionName + kKeyFreeDrawLineWidth, preferences_.objects.measure.free_draw_line_width).toFloat();
	preferences_.objects.measure.tape_line_multi_label = settings.value(kMeasureSectionName + kKeyTapeLineMultiLabel, preferences_.objects.measure.tape_line_multi_label).toBool();

	preferences_.objects.nerve.default_diameter = settings.value(kNerveSectionName + kKeyDefaultDiameter, preferences_.objects.nerve.default_diameter).toDouble();
	QString default_color = settings.value(kNerveSectionName + kKeyDefaultColor, preferences_.objects.nerve.default_color.name()).toString();
	preferences_.objects.nerve.default_color.setNamedColor(default_color);

	preferences_.objects.implant.rendering_type =
		(settings.value(kImplantSectionName + kKeyWire, false).toBool()) ? MeshRenderingType::Wire : MeshRenderingType::Volume;
	preferences_.objects.implant.alpha = settings.value(kImplantSectionName + kKeyAlpha, preferences_.objects.implant.alpha).toFloat();
	QString default_color_volume = settings.value(kImplantSectionName + kKeyDefaultColorVolume, preferences_.objects.implant.default_color_volume.name()).toString();
	preferences_.objects.implant.default_color_volume.setNamedColor(default_color_volume);
	QString default_color_wire = settings.value(kImplantSectionName + kKeyDefaultColorWire, preferences_.objects.implant.default_color_wire.name()).toString();
	preferences_.objects.implant.default_color_wire.setNamedColor(default_color_wire);
	QString selected_color_volume = settings.value(kImplantSectionName + kKeySelectedColorVolume, preferences_.objects.implant.selected_color_volume.name()).toString();
	preferences_.objects.implant.selected_color_volume.setNamedColor(selected_color_volume);
	QString selected_color_wire = settings.value(kImplantSectionName + kKeySelectedColorWire, preferences_.objects.implant.selected_color_wire.name()).toString();
	preferences_.objects.implant.selected_color_wire.setNamedColor(selected_color_wire);

	QString collided_color_volume = settings.value(kImplantSectionName + kKeyCollidedColorVolume, preferences_.objects.implant.collided_color_volume.name()).toString();
	QString collided_color_wire = settings.value(kImplantSectionName + kKeyCollidedColorWire, preferences_.objects.implant.collided_color_wire.name()).toString();

	// convert : 1.2.0.0 -> 1.3.0.0
	if (settings.contains(kImplantSectionName + kKeyCollisionColorVolume))
	{
		collided_color_volume = settings.value(kImplantSectionName + kKeyCollisionColorVolume, collided_color_volume).toString();
		settings.setValue(kImplantSectionName + kKeyCollidedColorVolume, collided_color_volume);
		settings.remove(kImplantSectionName + kKeyCollisionColorVolume);
	}

	if (settings.contains(kImplantSectionName + kKeyCollisionColorWire))
	{
		collided_color_wire = settings.value(kImplantSectionName + kKeyCollisionColorWire, collided_color_wire).toString();
		settings.setValue(kImplantSectionName + kKeyCollidedColorWire, collided_color_wire);
		settings.remove(kImplantSectionName + kKeyCollisionColorWire);
	}

	preferences_.objects.implant.collided_color_volume.setNamedColor(collided_color_volume);
	preferences_.objects.implant.collided_color_wire.setNamedColor(collided_color_wire);

	preferences_.objects.implant.collision_margin =
		settings.value(kImplantSectionName + kKeyCollisionMargin, preferences_.objects.implant.collision_margin).toDouble();
	preferences_.objects.implant.collision_margin_visible_on_2d_views =
		settings.value(kImplantSectionName + kKeyCollisionMarginVisibleOn2DViews, preferences_.objects.implant.collision_margin_visible_on_2d_views).toBool();
	preferences_.objects.implant.collision_margin_visible_on_3d_views =
		settings.value(kImplantSectionName + kKeyCollisionMarginVisibleOn3DViews, preferences_.objects.implant.collision_margin_visible_on_3d_views).toBool();
	preferences_.objects.implant.always_show_implant_id =
		settings.value(kImplantSectionName + kKeyAlwaysShowImplantId, preferences_.objects.implant.always_show_implant_id).toBool();
}

void GlobalPreferences::LoadAdvanced()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));
	
	preferences_.advanced.mpr.hide_mpr_views_on_maximized_vr_layout =
		settings.value(kMPRSectionName + kKeyHideMPRViewOnMaximizedVRLayout, preferences_.advanced.mpr.hide_mpr_views_on_maximized_vr_layout).toBool();
	preferences_.advanced.mpr.reorientation =
		(settings.value(kMPRSectionName + kKeyAutoOrientation, true).toBool()) ? Reorientation::Auto : Reorientation::Manual;
	preferences_.advanced.mpr.draw_arch =
		settings.value(kMPRSectionName + kKeyDrawArch, preferences_.advanced.mpr.draw_arch).toBool();
	preferences_.advanced.mpr.sagittal_direction =
		static_cast<Direction>(settings.value(kMPRSectionName + kKeySagittalDirection, static_cast<int>(preferences_.advanced.mpr.sagittal_direction)).toInt());
	preferences_.advanced.mpr.default_interval =
		settings.value(kMPRSectionName + kKeyDefaultInterval, preferences_.advanced.mpr.default_interval).toDouble();

	preferences_.advanced.panorama_view.move_axial_line_to_view_center =
		settings.value(kPanoramaViewSectionName + kKeyMoveAxialLineToViewCenter, preferences_.advanced.panorama_view.move_axial_line_to_view_center).toBool();

	preferences_.advanced.cross_section_view.thickness_increments =
		settings.value(kCrossSectionViewSectionName + kKeyThicknessIncrements, preferences_.advanced.cross_section_view.thickness_increments).toDouble();
	preferences_.advanced.cross_section_view.interval_increments =
		settings.value(kCrossSectionViewSectionName + kKeyIntervalIncrements, preferences_.advanced.cross_section_view.interval_increments).toDouble();
	preferences_.advanced.cross_section_view.default_interval =
		settings.value(kCrossSectionViewSectionName + kKeyDefaultInterval, preferences_.advanced.cross_section_view.default_interval).toDouble();
	preferences_.advanced.cross_section_view.flip_slices_across_the_arch_centerline =
		settings.value(kCrossSectionViewSectionName + kKeyFlipSlicesAcrossTheArchCenterline, preferences_.advanced.cross_section_view.flip_slices_across_the_arch_centerline).toBool();
	preferences_.advanced.cross_section_view.slide_as_set =
		settings.value(kCrossSectionViewSectionName + kKeySlideAsSet, preferences_.advanced.cross_section_view.slide_as_set).toBool();
	preferences_.advanced.cross_section_view.direction =
		static_cast<Direction>(settings.value(kCrossSectionViewSectionName + kKeyDirection, static_cast<int>(preferences_.advanced.cross_section_view.direction)).toInt());
	preferences_.advanced.cross_section_view.mouse_wheel_direction =
		static_cast<Direction>(settings.value(kCrossSectionViewSectionName + kKeyMouseWheelDirection, static_cast<int>(preferences_.advanced.cross_section_view.mouse_wheel_direction)).toInt());
	preferences_.advanced.cross_section_view.symmetry_by_implant =
		settings.value(kCrossSectionViewSectionName + kKeySymmetryByImplant, preferences_.advanced.cross_section_view.symmetry_by_implant).toBool();
	preferences_.advanced.cross_section_view.synchronize_angle_with_implant =
		settings.value(kCrossSectionViewSectionName + kKeySynchronizeAngleWithImplant, preferences_.advanced.cross_section_view.synchronize_angle_with_implant).toBool();
	preferences_.advanced.cross_section_view.shifted_by_interval =
		settings.value(kCrossSectionViewSectionName + kKeyShiftedByInterval, preferences_.advanced.cross_section_view.shifted_by_interval).toBool();
	preferences_.advanced.cross_section_view.default_layout =
		settings.value(kCrossSectionViewSectionName + kKeyDefaultLayout, preferences_.advanced.cross_section_view.default_layout).toSize();
	preferences_.advanced.cross_section_view.move_axial_line_to_view_center =
		settings.value(kCrossSectionViewSectionName + kKeyMoveAxialLineToViewCenter, preferences_.advanced.cross_section_view.move_axial_line_to_view_center).toBool();
		
	preferences_.advanced.panorama_axial_view.panorama_default_thickness =
		settings.value(kPanoramaAxialViewSectionName + kKeyPanoramaDefaultThickness, preferences_.advanced.panorama_axial_view.panorama_default_thickness).toDouble();
	preferences_.advanced.panorama_axial_view.panorama_thickness_increments =
		settings.value(kPanoramaAxialViewSectionName + kKeyPanoramaThicknessIncrements, preferences_.advanced.panorama_axial_view.panorama_thickness_increments).toDouble();
	preferences_.advanced.panorama_axial_view.panorama_default_range =
		settings.value(kPanoramaAxialViewSectionName + kKeyPanoramaDefaultRange, preferences_.advanced.panorama_axial_view.panorama_default_range).toDouble();

	preferences_.advanced.implant_view.translation_increments = 
		settings.value(kImplantViewSectionName + kKeyImplantTranslationIncrements, preferences_.advanced.implant_view.translation_increments).toInt();
	preferences_.advanced.implant_view.rotation_increments = 
		settings.value(kImplantViewSectionName + kKeyImplantRotationIncrements, preferences_.advanced.implant_view.rotation_increments).toInt();
	preferences_.advanced.implant_view.implant_handle_size_index =
		settings.value(kImplantViewSectionName + kKeyImplantHandleSize, preferences_.advanced.implant_view.implant_handle_size_index).toInt();
	preferences_.advanced.implant_view.implant_handle_width_index =
		settings.value(kImplantViewSectionName + kKeyImplantHandleWidth, preferences_.advanced.implant_view.implant_handle_width_index).toInt();
	preferences_.advanced.implant_view.cross_section_view_enable =
		settings.value(kImplantViewSectionName + kKeyCrossSectionViewEnable, preferences_.advanced.implant_view.cross_section_view_enable).toBool();

	preferences_.advanced.implant_view.implant_handle_size_index -= 1;
	if (preferences_.advanced.implant_view.implant_handle_size_index < 1)
		preferences_.advanced.implant_view.implant_handle_size_index = 0;
	else if (preferences_.advanced.implant_view.implant_handle_size_index > 6)
		preferences_.advanced.implant_view.implant_handle_size_index = 6;

	preferences_.advanced.implant_view.implant_handle_width_index -= 1;
	if (preferences_.advanced.implant_view.implant_handle_width_index < 1)
		preferences_.advanced.implant_view.implant_handle_width_index = 0;
	else if (preferences_.advanced.implant_view.implant_handle_width_index > 5)
		preferences_.advanced.implant_view.implant_handle_width_index = 5;

	preferences_.advanced.tmj.default_lateral_layout =
		settings.value(kTMJSectionName + kKeyDefaultLateralLayout, preferences_.advanced.tmj.default_lateral_layout).toSize();

#ifdef _WIN64
	preferences_.advanced.volume_rendering.quality =
		static_cast<GlobalPreferences::Quality2>(settings.value(kVolumeRenderingSectionName + kKeyVolumeRenderingQuality, static_cast<int>(preferences_.advanced.volume_rendering.quality)).toInt());
#endif

	preferences_.advanced.windowing.use_fixed_value = settings.value(kWindowingSectionName + kKeyUseFixedValue, preferences_.advanced.windowing.use_fixed_value).toBool();
	preferences_.advanced.windowing.fixed_level = settings.value(kWindowingSectionName + kKeyFixedLevel, preferences_.advanced.windowing.fixed_level).toInt();
	preferences_.advanced.windowing.fixed_width = settings.value(kWindowingSectionName + kKeyFixedWidth, preferences_.advanced.windowing.fixed_width).toInt();
}

void GlobalPreferences::loadPACSDefaultSetting()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	PACSDefaultSetting* dest = &preferences_.pacs_default_setting;
	QString preference = pacs::kPACSPreference + "/";

	preferences_.pacs.pacs_on = settings.value(pacs::kPACS + "/" + pacs::kPACSOn, preferences_.pacs.pacs_on).toBool();

	//MPR
	dest->mpr_view_list_swap = settings.value(preference + pacs::kMPRViewListSwap, dest->mpr_view_list_swap).toBool();
	dest->mpr_is_2d_vertical = settings.value(preference + pacs::kMPRIs2DVertical, dest->mpr_is_2d_vertical).toBool();
	dest->mpr_2d_angle = settings.value(preference + pacs::kMPR2DAngle, dest->mpr_2d_angle).toInt();
	dest->mpr_interval = settings.value(preference + pacs::kMPRInterval, dest->mpr_interval).toInt();
	dest->mpr_thickness = settings.value(preference + pacs::kMPRThickness, dest->mpr_thickness).toInt();
	dest->mpr_filter_level = settings.value(preference + pacs::kMPRFilterLevel, dest->mpr_filter_level).toInt();
	dest->mpr_is_3d_vertical = settings.value(preference + pacs::kMPRIs3DVertical, dest->mpr_is_3d_vertical).toBool();
	dest->mpr_is_dir_posterior = settings.value(preference + pacs::kMPRIsDirPosterior, dest->mpr_is_dir_posterior).toBool();
	dest->mpr_3d_angle = settings.value(preference + pacs::kMPR3DAngle, dest->mpr_3d_angle).toInt();
}

void GlobalPreferences::LoadPACSServerList()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.beginGroup(pacs::kPACSServer);
	int server_cnt = settings.value(pacs::kServerCnt).toInt();
	int select_num = settings.value(pacs::kSelectNum).toInt();
	settings.endGroup();

	if (server_cnt == 0)
	{
		return;
	}

	for (int i = 0; i < server_cnt; ++i)
	{
		settings.beginGroup(pacs::kPACSServerINFO + "_" + QString::number(i));
		{
			QString nickname = settings.value(pacs::kNickname).toString();
			QString ae_title = settings.value(pacs::kAETitle).toString();
			QString ip = settings.value(pacs::kIPAddress).toString();
			QString port = settings.value(pacs::kPort).toString();

			PACSServerInfo server_info(nickname, ae_title, ip, port);
			preferences_.pacs_server_list.server_list.emplace_back(server_info);
		}
		settings.endGroup();
	}

	preferences_.pacs_server_list.select_num = select_num;
}


void GlobalPreferences::SaveGeneral()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.setValue(kDatabaseSectionName + kKeyPort, preferences_.general.database.port);
	settings.setValue(kDatabaseSectionName + kKeyVersion, preferences_.general.database.version);

	settings.setValue(kNetworkSettingSectionName + kKeyUseWillmaster, preferences_.general.network_setting.use_willmaster);
	settings.setValue(kNetworkSettingSectionName + kKeyConnectionType, static_cast<int>(preferences_.general.network_setting.connection_type));
	settings.setValue(kNetworkSettingSectionName + kKeyIpAddress, preferences_.general.network_setting.ip_address);

	settings.setValue(kFilesSectionName + kKeyFavoriteOpenPaths, preferences_.general.files.favorite_open_paths);
	settings.setValue(kFilesSectionName + kKeyCapturePath, preferences_.general.files.capture_path);
	settings.setValue(kFilesSectionName + kKeyCapturePathWithPatientFolder, preferences_.general.files.capture_path_with_patient_folder);
	settings.setValue(kFilesSectionName + kKeySTLExportPath, preferences_.general.files.stl_export_path);

	settings.setValue(kInterfaceSectionName + kKeyLanguage, static_cast<int>(preferences_.general.interfaces.language));
	settings.setValue(kInterfaceSectionName + kKeyGUISize, static_cast<int>(preferences_.general.interfaces.gui_size));
	settings.setValue(kInterfaceSectionName + kKeyFontSize, static_cast<int>(preferences_.general.interfaces.font_size));
	settings.setValue(kInterfaceSectionName + kKeyMaximized, preferences_.general.interfaces.is_maximize);
	settings.setValue(kInterfaceSectionName + kKeyMaximizeType, preferences_.general.interfaces.maximize_type);

	settings.setValue(kDisplaySectionName + kKeyShowSliceNumbers, preferences_.general.display.show_slice_numbers);
	settings.setValue(kDisplaySectionName + kKeyShowRulers, preferences_.general.display.show_rulers);
	settings.setValue(kDisplaySectionName + kKeyGridSpacing, preferences_.general.display.grid_spacing);
}

void GlobalPreferences::SaveObject() 
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.setValue(kMeasureSectionName + kKeyLineColor, preferences_.objects.measure.line_color.name());
	settings.setValue(kMeasureSectionName + kKeyTextColor, preferences_.objects.measure.text_color.name());
	settings.setValue(kMeasureSectionName + kKeyTextSize, static_cast<int>(preferences_.objects.measure.text_size));
	settings.setValue(kMeasureSectionName + kKeyFreeDrawLineWidth, preferences_.objects.measure.free_draw_line_width);
	settings.setValue(kMeasureSectionName + kKeyTapeLineMultiLabel, preferences_.objects.measure.tape_line_multi_label);

	settings.setValue(kNerveSectionName + kKeyDefaultDiameter, preferences_.objects.nerve.default_diameter);
	settings.setValue(kNerveSectionName + kKeyDefaultColor, preferences_.objects.nerve.default_color.name());

	settings.setValue(kImplantSectionName + kKeyWire, static_cast<bool>(preferences_.objects.implant.rendering_type));
	settings.setValue(kImplantSectionName + kKeyAlpha, preferences_.objects.implant.alpha);
	settings.setValue(kImplantSectionName + kKeyDefaultColorVolume, preferences_.objects.implant.default_color_volume.name());
	settings.setValue(kImplantSectionName + kKeyDefaultColorWire, preferences_.objects.implant.default_color_wire.name());
	settings.setValue(kImplantSectionName + kKeySelectedColorVolume, preferences_.objects.implant.selected_color_volume.name());
	settings.setValue(kImplantSectionName + kKeySelectedColorWire, preferences_.objects.implant.selected_color_wire.name());
	settings.setValue(kImplantSectionName + kKeyCollidedColorVolume, preferences_.objects.implant.collided_color_volume.name());
	settings.setValue(kImplantSectionName + kKeyCollidedColorWire, preferences_.objects.implant.collided_color_wire.name());
	settings.setValue(kImplantSectionName + kKeyCollisionMargin, preferences_.objects.implant.collision_margin);
	settings.setValue(kImplantSectionName + kKeyCollisionMarginVisibleOn2DViews, preferences_.objects.implant.collision_margin_visible_on_2d_views);
	settings.setValue(kImplantSectionName + kKeyCollisionMarginVisibleOn3DViews, preferences_.objects.implant.collision_margin_visible_on_3d_views);
	settings.setValue(kImplantSectionName + kKeyAlwaysShowImplantId, preferences_.objects.implant.always_show_implant_id);
}

void GlobalPreferences::SaveAdvanced()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	settings.setValue(kMPRSectionName + kKeyHideMPRViewOnMaximizedVRLayout, preferences_.advanced.mpr.hide_mpr_views_on_maximized_vr_layout);
	settings.setValue(kMPRSectionName + kKeyAutoOrientation, (preferences_.advanced.mpr.reorientation == Reorientation::Auto) ? true : false);
	settings.setValue(kMPRSectionName + kKeyDefaultInterval, preferences_.advanced.mpr.default_interval);
#if 0 // 설정창에 없는 항목들은 저장하지 않음
	settings.setValue(kMPRSectionName + kKeyDrawArch, preferences_.advanced.mpr.draw_arch);
	settings.setValue(kMPRSectionName + kKeySagittalDirection, static_cast<int>(preferences_.advanced.mpr.sagittal_direction));
#endif

	settings.setValue(kCrossSectionViewSectionName + kKeyThicknessIncrements, preferences_.advanced.cross_section_view.thickness_increments);
	settings.setValue(kCrossSectionViewSectionName + kKeyIntervalIncrements, preferences_.advanced.cross_section_view.interval_increments);
	settings.setValue(kCrossSectionViewSectionName + kKeyDefaultInterval, preferences_.advanced.cross_section_view.default_interval);
	settings.setValue(kCrossSectionViewSectionName + kKeyFlipSlicesAcrossTheArchCenterline, preferences_.advanced.cross_section_view.flip_slices_across_the_arch_centerline);
#if 0 // 설정창에 없는 항목들은 저장하지 않음
	settings.setValue(kCrossSectionViewSectionName + kKeySlideAsSet, preferences_.advanced.cross_section_view.slide_as_set);
	settings.setValue(kCrossSectionViewSectionName + kKeyDirection, static_cast<int>(preferences_.advanced.cross_section_view.direction));
	settings.setValue(kCrossSectionViewSectionName + kKeyMouseWheelDirection, static_cast<int>(preferences_.advanced.cross_section_view.mouse_wheel_direction));
	settings.setValue(kCrossSectionViewSectionName + kKeySymmetryByImplant, preferences_.advanced.cross_section_view.symmetry_by_implant);
	settings.setValue(kCrossSectionViewSectionName + kKeySynchronizeAngleWithImplant, preferences_.advanced.cross_section_view.synchronize_angle_with_implant);
	settings.setValue(kCrossSectionViewSectionName + kKeyShiftedByInterval, preferences_.advanced.cross_section_view.shifted_by_interval);
#endif

	settings.setValue(kPanoramaAxialViewSectionName + kKeyPanoramaDefaultThickness, preferences_.advanced.panorama_axial_view.panorama_default_thickness);
	settings.setValue(kPanoramaAxialViewSectionName + kKeyPanoramaThicknessIncrements, preferences_.advanced.panorama_axial_view.panorama_thickness_increments);
	settings.setValue(kPanoramaAxialViewSectionName + kKeyPanoramaDefaultRange, preferences_.advanced.panorama_axial_view.panorama_default_range);

	settings.setValue(kImplantViewSectionName + kKeyImplantTranslationIncrements, preferences_.advanced.implant_view.translation_increments);
	settings.setValue(kImplantViewSectionName + kKeyImplantRotationIncrements, preferences_.advanced.implant_view.rotation_increments);
	settings.setValue(kImplantViewSectionName + kKeyImplantHandleSize, preferences_.advanced.implant_view.implant_handle_size_index + 1);
	settings.setValue(kImplantViewSectionName + kKeyImplantHandleWidth, preferences_.advanced.implant_view.implant_handle_width_index + 1);
	settings.setValue(kImplantViewSectionName + kKeyCrossSectionViewEnable, preferences_.advanced.implant_view.cross_section_view_enable);

	settings.setValue(kVolumeRenderingSectionName + kKeyVolumeRenderingQuality, static_cast<int>(preferences_.advanced.volume_rendering.quality));

	settings.setValue(kWindowingSectionName + kKeyUseFixedValue, preferences_.advanced.windowing.use_fixed_value);
	settings.setValue(kWindowingSectionName + kKeyFixedLevel, preferences_.advanced.windowing.fixed_level);
	settings.setValue(kWindowingSectionName + kKeyFixedWidth, preferences_.advanced.windowing.fixed_width);
}

void GlobalPreferences::SavePACSDefaultSetting()
{
	QSettings settings(ini_path_, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

	PACSDefaultSetting* dest = &preferences_.pacs_default_setting;
	QString preference = pacs::kPACSPreference + "/";

	settings.setValue(pacs::kPACS + "/" + pacs::kPACSOn, preferences_.pacs.pacs_on);

	//MPR
	settings.setValue(preference + pacs::kMPRViewListSwap, preferences_.pacs_default_setting.mpr_view_list_swap);
	settings.setValue(preference + pacs::kMPRIs2DVertical, preferences_.pacs_default_setting.mpr_is_2d_vertical);
	settings.setValue(preference + pacs::kMPR2DAngle, preferences_.pacs_default_setting.mpr_2d_angle);
	settings.setValue(preference + pacs::kMPRInterval, preferences_.pacs_default_setting.mpr_interval);
	settings.setValue(preference + pacs::kMPRThickness, preferences_.pacs_default_setting.mpr_thickness);
	settings.setValue(preference + pacs::kMPRFilterLevel, preferences_.pacs_default_setting.mpr_filter_level);
	settings.setValue(preference + pacs::kMPRIs3DVertical, preferences_.pacs_default_setting.mpr_is_3d_vertical);
	settings.setValue(preference + pacs::kMPRIsDirPosterior, preferences_.pacs_default_setting.mpr_is_dir_posterior);
	settings.setValue(preference + pacs::kMPR3DAngle, preferences_.pacs_default_setting.mpr_3d_angle);
}

QVariant GlobalPreferences::GetINIData(QString main, QString sub , QString default, QString path)
{
  QVariant return_value;
  QSettings settings(path, QSettings::IniFormat);
  settings.setIniCodec(QTextCodec::codecForName(ini_codec_name_));

  settings.beginGroup(main);
  return_value = settings.value(sub, default);
  settings.endGroup();

  return return_value;
}