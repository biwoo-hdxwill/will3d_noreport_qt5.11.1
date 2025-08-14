#include "project_io_flie.h"
#include <H5Cpp.h>
#include <iostream>

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ImageHeader.h"
#include "project_path_info.h"
#include "io_functions.h"

using namespace H5;
using namespace project;

ProjectIOFile::ProjectIOFile(const project::Purpose& purpose,
	const std::shared_ptr<H5::H5File>& file) :
	file_(file)
{
	if (purpose == project::Purpose::SAVE)
	{
		file_->createGroup(group::kTabFile);
	}
}

ProjectIOFile::~ProjectIOFile() {}

void ProjectIOFile::SaveMainVolume(CW3Image3D * vol)
{
	SaveVolume(vol, 0);
}

void ProjectIOFile::LoadMainVolume(CW3Image3D *& vol)
{
	LoadVolume(vol, 0);
}

void ProjectIOFile::SaveSecondVolume(CW3Image3D * vol)
{
	SaveVolume(vol, 1);
}

void ProjectIOFile::LoadSecondVolume(CW3Image3D *& vol)
{
	LoadVolume(vol, 1);
}

void ProjectIOFile::SaveVolume(CW3Image3D* vol, const int id)
{
	std::string root_group_name;
	std::string data_group_name;
	std::string header_group_name;

	if (id == 0)
	{
		root_group_name = group::kResMainVol;
		data_group_name = group::kMainVolData;
		header_group_name = group::kMainVolDCMHeader;
	}
	else if (id == 1)
	{
		root_group_name = group::kResSecondVol;
		data_group_name = group::kSecondVolData;
		header_group_name = group::kSecondVolDCMHeader;
	}
	else
	{
		return;
	}

	Group vol_group = file_->createGroup(root_group_name);
	// volume info save
	VolInfo vol_info;
	vol_info.width = vol->width();
	vol_info.height = vol->height();
	vol_info.depth = vol->depth();
	vol_info.min = vol->getMin();
	vol_info.max = vol->getMax();
	vol_info.pixel_spacing = vol->pixelSpacing();
	vol_info.slice_spacing = vol->sliceSpacing();
	vol_info.window_center = vol->windowCenter();
	vol_info.window_width = vol->windowWidth();
	vol_info.slope = vol->slope();
	vol_info.intercept = vol->intercept();
	vol_info.histo_size = vol->getHistoSize();
	vol_info.slice_loc_maxilla = vol->getSliceLoc().maxilla;
	vol_info.slice_loc_teeth = vol->getSliceLoc().teeth;
	vol_info.slice_loc_chin = vol->getSliceLoc().chin;
	vol_info.slice_loc_nose = vol->getSliceLoc().nose;
	vol_info.threshold_air = vol->getAirTissueThreshold();
	vol_info.threshold_tissue = vol->getTissueBoneThreshold();
	vol_info.threshold_bone = vol->getBoneTeethThreshold();

	CompType member_type(sizeof(VolInfo));
	member_type.insertMember(ds_member::kWidth,
		HOFFSET(VolInfo, width),
		PredType::NATIVE_UINT);
	member_type.insertMember(ds_member::kHeight,
		HOFFSET(VolInfo, height),
		PredType::NATIVE_UINT);
	member_type.insertMember(ds_member::kDepth,
		HOFFSET(VolInfo, depth),
		PredType::NATIVE_UINT);
	member_type.insertMember(ds_member::kMin,
		HOFFSET(VolInfo, min),
		PredType::NATIVE_USHORT);
	member_type.insertMember(ds_member::kMax,
		HOFFSET(VolInfo, max),
		PredType::NATIVE_USHORT);
	member_type.insertMember(ds_member::kPixelSpacing,
		HOFFSET(VolInfo, pixel_spacing),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kSliceSpacing,
		HOFFSET(VolInfo, slice_spacing),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kWindowCenter,
		HOFFSET(VolInfo, window_center),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kWindowWidth,
		HOFFSET(VolInfo, window_width),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kSlope,
		HOFFSET(VolInfo, slope),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kIntercept,
		HOFFSET(VolInfo, intercept),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kHistoSize,
		HOFFSET(VolInfo, histo_size),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kSliceLocMaxilla,
		HOFFSET(VolInfo, slice_loc_maxilla),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kSliceLocTeeth,
		HOFFSET(VolInfo, slice_loc_teeth),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kSliceLocChin,
		HOFFSET(VolInfo, slice_loc_chin),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kSliceLocNose,
		HOFFSET(VolInfo, slice_loc_nose),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kThresholdAir,
		HOFFSET(VolInfo, threshold_air),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kThresholdTissue,
		HOFFSET(VolInfo, threshold_tissue),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kThresholdBone,
		HOFFSET(VolInfo, threshold_bone),
		PredType::NATIVE_INT);

	DataSet dataset = vol_group.createDataSet(ds::kVolInfo, member_type, project::io::kDSScalar);
	dataset.write(&vol_info, member_type);
	dataset.close();

	// volume data save
	Group vol_data_group = file_->createGroup(data_group_name);
	hsize_t dim_vol[1] = { vol_info.width*vol_info.height };
	DataSpace space_vol(1, dim_vol);
	for (int d = 0; d < vol_info.depth; ++d)
	{
		dataset = vol_data_group.createDataSet(std::to_string(d), PredType::NATIVE_USHORT, space_vol);
		dataset.write(vol->getData()[d], PredType::NATIVE_USHORT);
	}
	dataset.close();

	// histogram save
	hsize_t dim_histogram[] = { (hsize_t)vol_info.histo_size };
	DataSpace space_histogram(1, dim_histogram);
	dataset = vol_group.createDataSet(ds::kVolHistogram, PredType::NATIVE_INT, space_histogram);
	dataset.write(vol->getHistogram(), PredType::NATIVE_INT);
	dataset.close();

	// DCM header save
	Group vol_header_group = file_->createGroup(header_group_name);
	CW3ImageHeader* header = vol->getHeader();
	for (const auto& data : header->getListCore())
	{
		if (data.second.isEmpty())
			continue;

		IOFunctions::WriteString(vol_header_group, data.first, data.second.toStdString());
	}

	if (id == 1)
	{
		// Second to first matrix save
		const glm::mat4& mat = vol->getSecondToFirst();
		vol_group = file_->openGroup(root_group_name);
		IOFunctions::WriteMatrix(vol_group, ds_member::kSecondToFirst, mat);
	}
}

void ProjectIOFile::LoadVolume(CW3Image3D*& vol, const int id)
{
	std::string root_group_name;
	std::string data_group_name;
	std::string header_group_name;

	if (id == 0)
	{
		root_group_name = group::kResMainVol;
		data_group_name = group::kMainVolData;
		header_group_name = group::kMainVolDCMHeader;
	}
	else if (id == 1)
	{
		root_group_name = group::kResSecondVol;
		data_group_name = group::kSecondVolData;
		header_group_name = group::kSecondVolDCMHeader;
	}
	else
	{
		return;
	}

	if (!file_->exists(root_group_name))
	{
		vol = nullptr;
		return;
	}

	Group volume_grp = file_->openGroup(root_group_name);

	// volume info load
	VolInfo vol_info;
	CompType member_type(sizeof(VolInfo));
	member_type.insertMember(ds_member::kWidth,
		HOFFSET(VolInfo, width),
		PredType::NATIVE_UINT);
	member_type.insertMember(ds_member::kHeight,
		HOFFSET(VolInfo, height),
		PredType::NATIVE_UINT);
	member_type.insertMember(ds_member::kDepth,
		HOFFSET(VolInfo, depth),
		PredType::NATIVE_UINT);
	member_type.insertMember(ds_member::kMin,
		HOFFSET(VolInfo, min),
		PredType::NATIVE_USHORT);
	member_type.insertMember(ds_member::kMax,
		HOFFSET(VolInfo, max),
		PredType::NATIVE_USHORT);
	member_type.insertMember(ds_member::kPixelSpacing,
		HOFFSET(VolInfo, pixel_spacing),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kSliceSpacing,
		HOFFSET(VolInfo, slice_spacing),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kWindowCenter,
		HOFFSET(VolInfo, window_center),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kWindowWidth,
		HOFFSET(VolInfo, window_width),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kSlope,
		HOFFSET(VolInfo, slope),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kIntercept,
		HOFFSET(VolInfo, intercept),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kHistoSize,
		HOFFSET(VolInfo, histo_size),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kSliceLocMaxilla,
		HOFFSET(VolInfo, slice_loc_maxilla),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kSliceLocTeeth,
		HOFFSET(VolInfo, slice_loc_teeth),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kSliceLocChin,
		HOFFSET(VolInfo, slice_loc_chin),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kSliceLocNose,
		HOFFSET(VolInfo, slice_loc_nose),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kThresholdAir,
		HOFFSET(VolInfo, threshold_air),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kThresholdTissue,
		HOFFSET(VolInfo, threshold_tissue),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kThresholdBone,
		HOFFSET(VolInfo, threshold_bone),
		PredType::NATIVE_INT);

	DataSet dataset = volume_grp.openDataSet(ds::kVolInfo);
	dataset.read(&vol_info, member_type);
	dataset.close();

	SliceLoc slice_loc;
	slice_loc.maxilla = vol_info.slice_loc_maxilla;
	slice_loc.teeth = vol_info.slice_loc_teeth;
	slice_loc.chin = vol_info.slice_loc_chin;
	slice_loc.nose = vol_info.slice_loc_nose;

	vol = new CW3Image3D(vol_info.width, vol_info.height, vol_info.depth);
	vol->setMinMax(vol_info.min, vol_info.max);
	vol->setPixelSpacing(vol_info.pixel_spacing);
	vol->setSliceSpacing(vol_info.slice_spacing);
	vol->setWindowing(vol_info.window_center, vol_info.window_width);
	vol->setSlope(vol_info.slope);
	vol->setIntercept(vol_info.intercept);
	vol->setSliceLoc(slice_loc);
	vol->setThreshold(vol_info.threshold_air, vol_info.threshold_tissue, vol_info.threshold_bone);

	// histogram load
	int* histogram = new int[vol_info.histo_size];
	dataset = volume_grp.openDataSet(ds::kVolHistogram);
	dataset.read(histogram, PredType::NATIVE_INT);
	dataset.close();
	vol->setHistogram(histogram, vol_info.histo_size);
	delete[] histogram;

	// volume data load
	unsigned short** dat = vol->getData();
	Group vol_data_group = file_->openGroup(data_group_name);
	for (int d = 0; d < vol_info.depth; ++d)
	{
		dataset = vol_data_group.openDataSet(std::to_string(d));
		dataset.read(vol->getData()[d], PredType::NATIVE_USHORT);
	}
	dataset.close();

	// DCM header load
	using namespace dcm::tags;
	std::map<std::string, QString> header;
	Group vol_dcm_header_group = file_->openGroup(header_group_name);
	std::string result = IOFunctions::IOFunctions::ReadString(vol_dcm_header_group, kPatientID);
	if (result.size() > 0) header[kPatientID] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kPatientName);
	if (result.size() > 0) header[kPatientName] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kPatientGender);
	if (result.size() > 0) header[kPatientGender] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kPatientAge);
	if (result.size() > 0) header[kPatientAge] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kPatientBirthDate);
	if (result.size() > 0) header[kPatientBirthDate] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kStudyInstanceUID);
	if (result.size() > 0) header[kStudyInstanceUID] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kSeriesInstanceUID);
	if (result.size() > 0) header[kSeriesInstanceUID] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kStudyDate);
	if (result.size() > 0) header[kStudyDate] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kSeriesDate);
	if (result.size() > 0) header[kSeriesDate] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kStudyTime);
	if (result.size() > 0) header[kStudyTime] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kSeriesTime);
	if (result.size() > 0) header[kSeriesTime] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kModality);
	if (result.size() > 0) header[kModality] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kKVP);
	if (result.size() > 0) header[kKVP] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kXRayTubeCurrent);
	if (result.size() > 0) header[kXRayTubeCurrent] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kExposureTime);
	if (result.size() > 0) header[kExposureTime] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kPixelSpacing);
	if (result.size() > 0) header[kPixelSpacing] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kSliceThickness);
	if (result.size() > 0) header[kSliceThickness] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kBitsAllocated);
	if (result.size() > 0) header[kBitsAllocated] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kRescaleIntercept);
	if (result.size() > 0) header[kRescaleIntercept] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kRescaleSlope);
	if (result.size() > 0) header[kRescaleSlope] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kRows);
	if (result.size() > 0) header[kRows] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kColumns);
	if (result.size() > 0) header[kColumns] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kWindowCenter);
	if (result.size() > 0) header[kWindowCenter] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kWindowWidth);
	if (result.size() > 0) header[kWindowWidth] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kStudyDescription);
	if (result.size() > 0) header[kStudyDescription] = QString::fromStdString(result);
	result = IOFunctions::ReadString(vol_dcm_header_group, kImagePositionPatient);
	if (result.size() > 0) header[kImagePositionPatient] = QString::fromStdString(result);

	for (auto &i : header)
	{
		if (i.first == std::string("Rows"))
		{
			i.second = QString::number(vol->height());
		}

		if (i.first == std::string("Columns"))
		{
			i.second = QString::number(vol->width());
		}
	}

	std::shared_ptr<CW3ImageHeader> image_header(new CW3ImageHeader(header));
	vol->setHeader(image_header);

	if (id == 1)
	{
		// Second to first matrix load
		glm::mat4 mat = IOFunctions::ReadMatrix(volume_grp, ds_member::kSecondToFirst);
		vol->setSecondToFirst(mat);
	}
}

void ProjectIOFile::SaveVolumeInfo(CW3Image3D * vol)
{
	LoadVolInfo vol_info;
	vol_info.range_start = vol->start_image_num();
	vol_info.range_end = vol->start_image_num() + vol->depth() - 1;
	vol_info.area_x = vol->start_image_x();
	vol_info.area_y = vol->start_image_y();
	vol_info.width = vol->width();
	vol_info.height = vol->height();

	CompType member_type(sizeof(LoadVolInfo));
	member_type.insertMember(ds_member::kRangeStart,
		HOFFSET(LoadVolInfo, range_start),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kRangeEnd,
		HOFFSET(LoadVolInfo, range_end),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kAreaX,
		HOFFSET(LoadVolInfo, area_x),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kAreaY,
		HOFFSET(LoadVolInfo, area_y),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kWidth,
		HOFFSET(LoadVolInfo, width),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kHeight,
		HOFFSET(LoadVolInfo, height),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kPixelSpacing,
		HOFFSET(LoadVolInfo, pixel_spacing),
		PredType::NATIVE_FLOAT);

	Group file_tab_group = file_->openGroup(group::kTabFile);
	DataSet dataset = file_tab_group.createDataSet(ds::kVolInfo, member_type,
		project::io::kDSScalar);
	dataset.write(&vol_info, member_type);
	dataset.close();
}

void ProjectIOFile::LoadVolumeInfo(project::LoadVolInfo& vol_info)
{
	CompType member_type(sizeof(LoadVolInfo));
	member_type.insertMember(ds_member::kRangeStart,
		HOFFSET(LoadVolInfo, range_start),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kRangeEnd,
		HOFFSET(LoadVolInfo, range_end),
		PredType::NATIVE_FLOAT);
	member_type.insertMember(ds_member::kAreaX,
		HOFFSET(LoadVolInfo, area_x),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kAreaY,
		HOFFSET(LoadVolInfo, area_y),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kWidth,
		HOFFSET(LoadVolInfo, width),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kHeight,
		HOFFSET(LoadVolInfo, height),
		PredType::NATIVE_INT);
	member_type.insertMember(ds_member::kPixelSpacing,
		HOFFSET(LoadVolInfo, pixel_spacing),
		PredType::NATIVE_FLOAT);

	Group file_tab_group = file_->openGroup(group::kTabFile);
	DataSet dataset = file_tab_group.openDataSet(ds::kVolInfo);
	dataset.read(&vol_info, member_type);
	dataset.close();
}
