#pragma once
/**=================================================================================================

Project:		Resource
File:			measure_resource.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-10-31
Last modify: 	2018-10-31

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <memory>
#include <map>
#include <vector>
#include <qpoint.h>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/define_measure.h"
#include "resource_global.h"

class MeasureData;

class RESOURCE_EXPORT MeasureResource {
public:
	MeasureResource();
	~MeasureResource();

	MeasureResource(const MeasureResource&) = delete;
	MeasureResource& operator=(const MeasureResource&) = delete;

private:
	typedef std::vector< std::shared_ptr< MeasureData > > MeasureList;

public:
	void AddMeasureData(const common::ViewTypeID& view_type,
						const std::shared_ptr< MeasureData >& data);
	void DeleteMeasureData(const common::ViewTypeID& view_type,
						   const unsigned int& measure_id);
	void DeleteMeasureDatas(const common::ViewTypeID& view_type);

	std::shared_ptr< MeasureData >& GetMeasureData(const common::ViewTypeID& view_type,
												   const unsigned int& measure_id);
	void GetMeasureList(const common::ViewTypeID& view_type,
						std::vector<std::weak_ptr<MeasureData>>* measure_list);
	void GetMeasureDatas(common::measure::MeasureDataContainer* measure_datas) const;

	void SetMeasureScale(const common::ViewTypeID& view_type, const float& scale);
	void SetMeasurePixelPitch(const common::ViewTypeID& view_type, const float& pixel_pitch);

private:
	std::map< common::ViewTypeID, MeasureList > datas_;
};

