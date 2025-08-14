#pragma once
/*=========================================================================

File:			project_io_measure_resource.h
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-11-15
Last modify:	2018-07-12

=========================================================================*/
#include <memory>
#include <H5Cpp.h>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/define_measure.h"

#include "datatypes.h"
#include "w3projectio_global.h"
namespace project {
typedef struct _MeasureViewParams MeasureViewInfo;
}

class MeasureData;

class W3PROJECTIO_EXPORT ProjectIOMeasureResource {
public:
	ProjectIOMeasureResource(const project::Purpose& purpose,
							 const std::shared_ptr<H5::H5File>& file);
	~ProjectIOMeasureResource();

	ProjectIOMeasureResource(const ProjectIOMeasureResource&) = delete;
	ProjectIOMeasureResource& operator=(const ProjectIOMeasureResource&) = delete;

public:
	void SaveMeasureCount(int total_count);
	void SaveMeasure(const common::ViewTypeID& view_type, const MeasureData& measure_data);
	void SaveViewParamsCount(int total_count);
	void SaveViewParams(const TabType& tab_type, const common::ViewTypeID& view_type,
						const common::measure::ViewInfo& view_info);
	void SaveCounterparts(const TabType& mpr_counterpart,
						  const TabType& pano_conuterpart);

	int LoadMeasureCount();
	void LoadMeasure(common::ViewTypeID* view_type, MeasureData*& measure_data);
	int LoadViewParamsCount();
	void LoadViewParams(TabType& tab_type, common::ViewTypeID& view_type,
						common::measure::ViewInfo& view_info);
	void LoadCounterparts(TabType& mpr_conterpart,
						  TabType& pano_conterpart);

	bool IsValid();

private:
	std::shared_ptr<H5::H5File> file_;
	int curr_count_measure_ = 0;
	int curr_count_view_ = 0;
	H5::CompType vec3_type_;
	H5::CompType measure_info_type_;
	H5::CompType view_info_type_;
};

