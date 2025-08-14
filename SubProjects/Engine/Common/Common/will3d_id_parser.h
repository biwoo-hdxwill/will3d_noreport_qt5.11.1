#pragma once
/**=================================================================================================

Project: 			UITools
File:				will3d_id_parser.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-10-29
Last modify:		2018-10-29

 *===============================================================================================**/
#include <QString>

#include "common_global.h"

#include "W3Enum.h"

namespace common
{
	enum ViewTypeID;
	namespace measure
	{
		enum class MeasureType;
	}  // end of namespace measure
}  // end of namespace common

class COMMON_EXPORT Will3DIDParser
{
public:
	Will3DIDParser();
	~Will3DIDParser();

public:
	static void ResetMeasureIDs();

	/**********************************************************************************************
	Gets measure identifier.
	@return	MeasureResourceMgr 클래스에서 Measure 생성 시 할당하는 함수.
	 **********************************************************************************************/
	static unsigned int GetMeasureID();

	/**********************************************************************************************
	Gets annotation note identifier.
	@return	MeasureResourceMgr 클래스에서 Measure - annotation 생성 시
	할당하는 함수.
	 **********************************************************************************************/
	static unsigned int GetMeasureNoteID();

	/**********************************************************************************************
	Gets measure profile identifier.
	@return	MeasureResourceMgr 클래스에서 Measure - profile 생성 시 할당하는
	함수.
	 **********************************************************************************************/
	static unsigned int GetMeasureProfileID();

	static QString GetMeasureTypeText(const common::measure::MeasureType& type);
	static QString GetMeasurePositionText(const common::ViewTypeID& view_type);

private:
	static unsigned int measure_id_;
	static unsigned int measure_note_id_;
	static unsigned int measure_profile_id_;
};
