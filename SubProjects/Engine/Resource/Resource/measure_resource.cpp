#include "measure_resource.h"
#include <qstring.h>
#include "../../Common/Common/W3Logger.h"
#include "../include/measure_data.h"

MeasureResource::MeasureResource() {}

MeasureResource::~MeasureResource() {}

void MeasureResource::AddMeasureData(const common::ViewTypeID& view_type,
	const std::shared_ptr< MeasureData >& data)
{
	// view_type 하위의 Measure List가 없는 경우
	if (datas_.find(view_type) == datas_.end())
	{
		MeasureList measure_list = { data };
		datas_.insert(std::pair<common::ViewTypeID, MeasureList>(view_type, measure_list));
	}
	else
	{
		datas_[view_type].push_back(data);
	}
}

void MeasureResource::DeleteMeasureData(const common::ViewTypeID & view_type,
	const unsigned int & measure_id)
{
	if (datas_.find(view_type) == datas_.end())
	{
		QString err_msg("MeasureResource::DeleteMeasureData: measure list does not exist");
		err_msg += " - view_type: " + QString::number((int)view_type);
		common::Logger::instance()->Print(common::LogType::ERR, err_msg.toStdString());
	}

	MeasureList& list = datas_[view_type];
	for (int idx = 0; idx < list.size(); ++idx)
	{
		if (list[idx]->id() == measure_id)
		{
			list.erase(list.begin() + idx);
			return;
		}
	}

	QString err_msg("MeasureResource::DeleteMeasureData: measure data does not exist");
	err_msg += " - view_type: " + QString::number((int)view_type)
		+ " / measure_id: " + QString::number(measure_id);
	common::Logger::instance()->Print(common::LogType::ERR, err_msg.toStdString());
}

void MeasureResource::DeleteMeasureDatas(const common::ViewTypeID & view_type)
{
	if (datas_.find(view_type) == datas_.end())
	{
		return;
	}

	datas_[view_type].clear();
}

std::shared_ptr< MeasureData >&
MeasureResource::GetMeasureData(const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
	if (datas_.find(view_type) == datas_.end())
	{
		QString err_msg("MeasureResource::GetMeasureData: measure list does not exist");
		err_msg += "view_type: " + QString::number((int)view_type);
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, err_msg.toStdString());
	}

	MeasureList& list = datas_[view_type];
	for (std::shared_ptr< MeasureData >& data : list)
	{
		if (data->id() == measure_id)
			return data;
	}

	QString err_msg("MeasureResource::GetMeasureData: measure data does not exist");
	err_msg += "view_type: " + QString::number((int)view_type)
		+ "measure_id: " + QString::number(measure_id);
	common::Logger::instance()->PrintAndAssert(common::LogType::ERR, err_msg.toStdString());
	return datas_[view_type][measure_id]; // 진행되지 않는 구문
}

// Measure List 없으면 없는대로 받는쪽에서 판단한다.
void MeasureResource::GetMeasureList(const common::ViewTypeID & view_type,
	std::vector<std::weak_ptr<MeasureData>>* measure_list)
{
	MeasureList& list = datas_[view_type];
	measure_list->reserve(list.size());
	for (std::shared_ptr< MeasureData >& data : list)
	{
		std::weak_ptr<MeasureData> w_data = data;
		measure_list->push_back(w_data);
	}
}

void MeasureResource::GetMeasureDatas(common::measure::MeasureDataContainer* measure_datas) const
{
	for (const auto& data : datas_)
	{
		std::vector< std::weak_ptr<MeasureData>> measure_data;
		measure_data.reserve(data.second.size());
		for (const std::shared_ptr< MeasureData >& md : data.second)
		{
			std::weak_ptr<MeasureData> w_data = md;
			measure_data.push_back(w_data);
		}

		measure_datas->insert(
			std::pair < common::ViewTypeID, std::vector< std::weak_ptr<MeasureData>> >(data.first, measure_data));
	}
}

void MeasureResource::SetMeasureScale(const common::ViewTypeID & view_type, const float & scale)
{
	if (datas_.find(view_type) == datas_.end())
		return;

	MeasureList& list = datas_[view_type];
	for (std::shared_ptr< MeasureData >& data : list)
	{
		data->set_scale(scale);
	}
}

void MeasureResource::SetMeasurePixelPitch(const common::ViewTypeID & view_type, const float & pixel_pitch)
{
	if (datas_.find(view_type) == datas_.end())
		return;

	MeasureList& list = datas_[view_type];
	for (std::shared_ptr< MeasureData >& data : list)
	{
		data->set_pixel_pitch(pixel_pitch);
	}
}
