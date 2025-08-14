#include "project_io_pano_engine.h"
#include <H5Cpp.h>

#include "../../Resource/Resource/cross_section_resource.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/Resource/sagittal_resource.h"

#include "io_functions.h"
#include "project_path_info.h"

#include <QDir>
#include <QDebug>

using namespace H5;
using namespace project;

ProjectIOPanoEngine::ProjectIOPanoEngine(
    const project::Purpose& purpose, const std::shared_ptr<H5::H5File>& file)
    : file_(file) {
  if (purpose == project::Purpose::SAVE) {
    file_->createGroup(group::kResPanoEngine);
    file_->createGroup(group::kResPanoMaxilla);
    file_->createGroup(group::kResPanoMandible);
    file_->createGroup(group::kNerveParams);
  }
  // 해당 리소스가 초기화되었을 때 만들어야 함
  // file_->createGroup(group::kResCrossSection);
  // file_->createGroup(group::kResSagittal);
}

ProjectIOPanoEngine::~ProjectIOPanoEngine() {}

void ProjectIOPanoEngine::SavePanoROI(float top, float bottom, float slice) {
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  IOFunctions::WriteVec3(pano_eng_grp, ds::kPanoROI,
                         glm::vec3(top, bottom, slice));
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::SaveReoriMatrix(const ArchTypeID& arch_type,
                                          const glm::mat4& reori) {
  std::string ds_name = arch_type == ArchTypeID::ARCH_MANDLBLE
                            ? ds::kPanoReoriMatMandible
                            : ds::kPanoReoriMatMaxilla;
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  IOFunctions::WriteMatrix(pano_eng_grp, ds_name, reori);
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::SaveImplant3DMVP(const glm::mat4& mvp) {
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  IOFunctions::WriteMatrix(pano_eng_grp, ds::kImplant3DMVP, mvp);
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::SaveCSShiftedValue(float shifted_value) {
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  IOFunctions::WriteFloat(pano_eng_grp, ds::kCSShiftedValue, shifted_value);
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::SaveCurrArchType(const ArchTypeID& arch_type) {
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  IOFunctions::WriteInt(pano_eng_grp, ds::kPanoCurrArchType, (int)arch_type);
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::SavePanoCtrlPoints(
    const ArchTypeID& arch_type,
    const std::vector<glm::vec3>& pano_ctrl_points) {
  std::string kCurrArchID = arch_type == ArchTypeID::ARCH_MAXILLA
                                ? group::kResPanoMaxilla
                                : group::kResPanoMandible;
  Group pano_grp = file_->openGroup(kCurrArchID);
  IOFunctions::WriteVec3List(pano_grp, ds::kPanoCtrlPoints, pano_ctrl_points);
  pano_grp.close();
}

void ProjectIOPanoEngine::SavePanoShiftValue(const ArchTypeID& arch_type,
                                             const float& pano_shift_value) {
  std::string kCurrArchID = arch_type == ArchTypeID::ARCH_MAXILLA
                                ? group::kResPanoMaxilla
                                : group::kResPanoMandible;
  Group pano_grp = file_->openGroup(kCurrArchID);
  IOFunctions::WriteFloat(pano_grp, ds::kPanoShiftedValue, pano_shift_value);
  pano_grp.close();
}

void ProjectIOPanoEngine::SaveNervePoints(
    const std::map<int, std::vector<glm::vec3>>& nerve_ctrl_points,
    const std::map<int, std::vector<glm::vec3>>& nerve_spline_points) {
  Group nerves_grp = file_->createGroup(group::kResNerve);
  int index = 0;
  for (const auto& nerve : nerve_ctrl_points) {
    const std::string kNerveIndex = std::to_string(index++);
    Group nerve_grp = nerves_grp.createGroup(kNerveIndex);
    IOFunctions::WriteInt(nerve_grp, ds::kNerveID, nerve.first);
    IOFunctions::WriteVec3List(nerve_grp, ds::kNerveCtrlPoints, nerve.second);
    nerve_grp.close();
  }

  index = 0;
  for (const auto& nerve : nerve_spline_points) {
    const std::string kNerveIndex = std::to_string(index++);
    Group nerve_grp = nerves_grp.openGroup(kNerveIndex);
    IOFunctions::WriteVec3List(nerve_grp, ds::kNerveSplinePoints, nerve.second);
    nerve_grp.close();
  }

  IOFunctions::WriteInt(nerves_grp, ds::kNerveTotalCount, index);
  nerves_grp.close();
}

void ProjectIOPanoEngine::SaveNerveParams(const int& idx, const int& nerve_id,
                                          int color_r, int color_g, int color_b,
                                          bool visible, float radius,
                                          double diameter_mm) {
  Group nerves_grp = file_->openGroup(group::kNerveParams);
  NerveParams param;
  param.color_r = color_r;
  param.color_g = color_g;
  param.color_b = color_b;
  param.visible = visible;
  param.diameter_mm = diameter_mm;
  param.radius = radius;
  param.id = nerve_id;

  CompType member_type(sizeof(NerveParams));
  member_type.insertMember(ds_member::kX, HOFFSET(NerveParams, color_r),
                           PredType::NATIVE_INT);
  member_type.insertMember(ds_member::kY, HOFFSET(NerveParams, color_g),
                           PredType::NATIVE_INT);
  member_type.insertMember(ds_member::kZ, HOFFSET(NerveParams, color_b),
                           PredType::NATIVE_INT);
  member_type.insertMember(ds_member::kVisible, HOFFSET(NerveParams, visible),
                           PredType::NATIVE_HBOOL);
  member_type.insertMember(ds_member::kRadius, HOFFSET(NerveParams, radius),
                           PredType::NATIVE_FLOAT);
  member_type.insertMember(ds_member::kDiameter,
                           HOFFSET(NerveParams, diameter_mm),
                           PredType::NATIVE_DOUBLE);
  member_type.insertMember(ds_member::kID, HOFFSET(NerveParams, id),
                           PredType::NATIVE_INT);

  std::string kNerveID = std::to_string(idx);
  DataSet dataset =
      nerves_grp.createDataSet(kNerveID, member_type, project::io::kDSScalar);
  dataset.write(&param, member_type);
  dataset.close();

  nerves_grp.close();
}

void ProjectIOPanoEngine::SaveNerveCount(const int& nerve_cnt) {
  Group nerves_grp = file_->openGroup(group::kNerveParams);
  IOFunctions::WriteInt(nerves_grp, ds::kNerveTotalCount, nerve_cnt);
  nerves_grp.close();
}

void ProjectIOPanoEngine::SaveImplantResource(const ImplantResource& imp_res, QString appfile_path) {
  Group imp_grp = file_->createGroup(group::kResImplant);

  std::string memo_string = imp_res.memo().toLocal8Bit();
  IOFunctions::WriteString(imp_grp, ds::kImplantMemo, memo_string);

  const auto& implant_datas = imp_res.data();
  if (implant_datas.empty())
  {
	  return;
  }

  IOFunctions::WriteInt(imp_grp, ds::kSelectedImplantID, imp_res.selected_implant_id());

  Group imp_data_grp = file_->createGroup(group::kImplantData);
  IOFunctions::WriteInt(imp_data_grp, ds::kImplantCount, static_cast<int>(implant_datas.size()));
  int imp_data_cnt = 0;
  for (const auto& implant_data : implant_datas) {
    const auto& data = implant_data.second;
    const std::string kImpDataGrpID = std::to_string(imp_data_cnt++);
    Group content_grp = imp_data_grp.createGroup(kImpDataGrpID);
    IOFunctions::WriteBool(content_grp, ds::kImplantVisible,
                           data->is_visible());
    IOFunctions::WriteUInt(content_grp, ds::kImplantID, data->id());
    IOFunctions::WriteFloat(content_grp, ds::kImplantPlatformDiameter, data->platform_diameter());
	IOFunctions::WriteFloat(content_grp, ds::kImplantCustomApicalDiameter, data->custom_apical_diameter());
	IOFunctions::WriteFloat(content_grp, ds::kImplantDiameter, data->diameter());
    IOFunctions::WriteFloat(content_grp, ds::kImplantTotalLength, data->total_length());
	IOFunctions::WriteFloat(content_grp, ds::kImplantLength, data->length());
	IOFunctions::WriteString(content_grp, ds::kImplantSubCategory, data->sub_category().toLocal8Bit().constData());

    std::string path = data->file_path().toLocal8Bit().constData();
    IOFunctions::WriteString(content_grp, ds::kImplantPath, path);

    std::string manufacturer = data->manufacturer().toLocal8Bit().constData();
    IOFunctions::WriteString(content_grp, ds::kImplantManufacturer,
                             manufacturer);

    std::string product = data->product().toLocal8Bit().constData();
    IOFunctions::WriteString(content_grp, ds::kImplantProduct, product);

    IOFunctions::WriteVec3(content_grp, ds::kImplantPosInVol,
                           data->position_in_vol());
    IOFunctions::WriteMatrix(content_grp, ds::kImplantRotInVol,
                             data->rotate_in_vol());
	IOFunctions::WriteMatrix(content_grp, ds::kImplantTransInVol, data->translate_in_vol());
    content_grp.close();

	//20250123 LIN
	if (appfile_path != "")
	{
		//20250123 LIN save implant to AppFiles
		QDir cur_path = QDir::currentPath();
		QString cur_implant_path = cur_path.absolutePath() + "/" + data->file_path();
		QString target_implant_path = appfile_path + data->file_path();
		QDir().mkpath(QFileInfo(target_implant_path).absolutePath());
		if (QFile::exists(target_implant_path)) {
			QFile::remove(target_implant_path);
		}
		if (QFile::copy(cur_implant_path, target_implant_path)) {
			qDebug() << "File copied successfully to:" << target_implant_path;
		}
		else {
			qDebug() << "Failed to copy the file.";
		}
	}
  }
  imp_data_grp.close();
  imp_grp.close();
}

void ProjectIOPanoEngine::LoadPanoROI(float& top, float& bottom, float& slice) {
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  glm::vec3 result = IOFunctions::ReadVec3(pano_eng_grp, ds::kPanoROI);
  top = result.x;
  bottom = result.y;
  slice = result.z;
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::LoadReoriMatrix(const ArchTypeID& arch_type,
                                          glm::mat4& reorientation) {
  std::string ds_name = arch_type == ArchTypeID::ARCH_MANDLBLE
                            ? ds::kPanoReoriMatMandible
                            : ds::kPanoReoriMatMaxilla;
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  reorientation = IOFunctions::ReadMatrix(pano_eng_grp, ds_name);
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::LoadImplant3DMVP(glm::mat4& mvp) {
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  mvp = IOFunctions::ReadMatrix(pano_eng_grp, ds::kImplant3DMVP);
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::LoadCSShiftedValue(float& shifted_value) {
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  shifted_value = IOFunctions::ReadFloat(pano_eng_grp, ds::kCSShiftedValue);
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::LoadCurrArchType(ArchTypeID& arch_type) {
  Group pano_eng_grp = file_->openGroup(group::kResPanoEngine);
  arch_type =
      (ArchTypeID)IOFunctions::ReadInt(pano_eng_grp, ds::kPanoCurrArchType);
  pano_eng_grp.close();
}

void ProjectIOPanoEngine::LoadPanoCtrlPoints(
    const ArchTypeID& arch_type, std::vector<glm::vec3>& pano_ctrl_points) {
  std::string kCurrArchID = arch_type == ArchTypeID::ARCH_MAXILLA
                                ? group::kResPanoMaxilla
                                : group::kResPanoMandible;
  if (!file_->exists(kCurrArchID)) return;

  Group pano_grp = file_->openGroup(kCurrArchID);
  pano_ctrl_points = IOFunctions::ReadVec3List(pano_grp, ds::kPanoCtrlPoints);
  pano_grp.close();
}

void ProjectIOPanoEngine::LoadPanoShiftValue(const ArchTypeID& arch_type,
                                             float& pano_shift_value) {
  std::string kCurrArchID = arch_type == ArchTypeID::ARCH_MAXILLA
                                ? group::kResPanoMaxilla
                                : group::kResPanoMandible;
  if (!file_->exists(kCurrArchID)) return;

  Group pano_grp = file_->openGroup(kCurrArchID);
  pano_shift_value = IOFunctions::ReadFloat(pano_grp, ds::kPanoShiftedValue);
  pano_grp.close();
}

void ProjectIOPanoEngine::LoadNervePoints(
    std::map<int, std::vector<glm::vec3>>& nerve_ctrl_points,
    std::map<int, std::vector<glm::vec3>>& nerve_spline_points) {
  if (!file_->exists(group::kResNerve)) return;

  Group nerves_grp = file_->openGroup(group::kResNerve);
  int nerve_count = IOFunctions::ReadInt(nerves_grp, ds::kNerveTotalCount);

  for (int index = 0; index < nerve_count; ++index) {
    const std::string kNerveIndex = std::to_string(index);
    Group nerve_grp = nerves_grp.openGroup(kNerveIndex);
    int nerve_id = IOFunctions::ReadInt(nerve_grp, ds::kNerveID);
    nerve_ctrl_points[nerve_id] =
        IOFunctions::ReadVec3List(nerve_grp, ds::kNerveCtrlPoints);
    nerve_grp.close();
  }

  for (int index = 0; index < nerve_count; ++index) {
    const std::string kNerveIndex = std::to_string(index);
    Group nerve_grp = nerves_grp.openGroup(kNerveIndex);
    int nerve_id = IOFunctions::ReadInt(nerve_grp, ds::kNerveID);
    nerve_spline_points[nerve_id] =
        IOFunctions::ReadVec3List(nerve_grp, ds::kNerveSplinePoints);
    nerve_grp.close();
  }

  nerves_grp.close();
}

void ProjectIOPanoEngine::LoadNerveParams(const int& idx, int& nerve_id,
                                          int& color_r, int& color_g,
                                          int& color_b, bool& visible,
                                          float& radius, double& diameter_mm) {
  Group nerves_grp = file_->openGroup(group::kNerveParams);
  std::string kNerveID = std::to_string(idx);
  if (!nerves_grp.exists(kNerveID)) return;

  CompType member_type(sizeof(NerveParams));
  member_type.insertMember(ds_member::kX, HOFFSET(NerveParams, color_r),
                           PredType::NATIVE_INT);
  member_type.insertMember(ds_member::kY, HOFFSET(NerveParams, color_g),
                           PredType::NATIVE_INT);
  member_type.insertMember(ds_member::kZ, HOFFSET(NerveParams, color_b),
                           PredType::NATIVE_INT);
  member_type.insertMember(ds_member::kVisible, HOFFSET(NerveParams, visible),
                           PredType::NATIVE_HBOOL);
  member_type.insertMember(ds_member::kRadius, HOFFSET(NerveParams, radius),
                           PredType::NATIVE_FLOAT);
  member_type.insertMember(ds_member::kDiameter,
                           HOFFSET(NerveParams, diameter_mm),
                           PredType::NATIVE_DOUBLE);
  member_type.insertMember(ds_member::kID, HOFFSET(NerveParams, id),
                           PredType::NATIVE_INT);

  DataSet dataset = nerves_grp.openDataSet(kNerveID);
  NerveParams param;
  dataset.read(&param, member_type);
  dataset.close();
  color_r = param.color_r;
  color_g = param.color_g;
  color_b = param.color_b;
  visible = param.visible;
  radius = param.radius;
  diameter_mm = param.diameter_mm;
  nerve_id = param.id;

  nerves_grp.close();
}

void ProjectIOPanoEngine::LoadNerveCount(int& nerve_cnt) {
  if (!file_->exists(group::kNerveParams)) return;
  Group nerves_grp = file_->openGroup(group::kNerveParams);
  nerve_cnt = IOFunctions::ReadInt(nerves_grp, ds::kNerveTotalCount);
  nerves_grp.close();
}

void ProjectIOPanoEngine::LoadImplantResource(
    project::ImplantResParams& imp_resource) {
  if (!file_->exists(group::kResImplant)) return;

  Group imp_grp = file_->openGroup(group::kResImplant);

  std::string memo = IOFunctions::ReadString(imp_grp, ds::kImplantMemo);
  imp_resource.memo = QString::fromLocal8Bit(memo.c_str());

  if (!file_->exists(group::kImplantData))
  {
	  return;
  }

  imp_resource.selected_id = IOFunctions::ReadInt(imp_grp, ds::kSelectedImplantID);

  Group imp_data_grp = file_->openGroup(group::kImplantData);
  int total_implant_count =
      IOFunctions::ReadInt(imp_data_grp, ds::kImplantCount);
  for (int imp_data_cnt = 0; imp_data_cnt < total_implant_count;
       ++imp_data_cnt) {
    const std::string kImpDataGrpID = std::to_string(imp_data_cnt);
    Group content_grp = imp_data_grp.openGroup(kImpDataGrpID);
    ImplantDataParams data;

    data.visible = IOFunctions::ReadBool(content_grp, ds::kImplantVisible);
    data.id = IOFunctions::ReadUInt(content_grp, ds::kImplantID);
    data.platform_diameter = IOFunctions::ReadFloat(content_grp, ds::kImplantPlatformDiameter);
	data.custom_apical_diameter = IOFunctions::ReadFloat(content_grp, ds::kImplantCustomApicalDiameter);
	data.diameter = IOFunctions::ReadFloat(content_grp, ds::kImplantDiameter);
    data.total_length = IOFunctions::ReadFloat(content_grp, ds::kImplantTotalLength);
	data.length = IOFunctions::ReadFloat(content_grp, ds::kImplantLength);
	data.sub_category = QString::fromLocal8Bit(IOFunctions::ReadString(content_grp, ds::kImplantSubCategory).c_str());

    std::string path_str =
        IOFunctions::ReadString(content_grp, ds::kImplantPath);
    data.path = QString::fromLocal8Bit(path_str.c_str());

    std::string man_str =
        IOFunctions::ReadString(content_grp, ds::kImplantManufacturer);
    data.manufacturer = QString::fromLocal8Bit(man_str.c_str());

    std::string prod_str =
        IOFunctions::ReadString(content_grp, ds::kImplantProduct);
    data.product = QString::fromLocal8Bit(prod_str.c_str());

    data.pos_in_vol = IOFunctions::ReadVec3(content_grp, ds::kImplantPosInVol);
    data.rot_in_vol =
        IOFunctions::ReadMatrix(content_grp, ds::kImplantRotInVol);
	data.trans_in_vol = IOFunctions::ReadMatrix(content_grp, ds::kImplantTransInVol);
    content_grp.close();
    imp_resource.datas.push_back(data);
  }

  imp_data_grp.close();
  imp_grp.close();
}
