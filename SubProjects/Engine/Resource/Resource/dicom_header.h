#pragma once
/*=========================================================================
Copyright (c) 2017 All rights reserved by HDXWILL.

File:			dicom_header.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			Seo Seok Man
First Date:		2017-11-23
Modify Date:	2017-11-23
Version:		1.0

=========================================================================*/
#include <vector>
#include <map>

#include <qstring.h>

#include "resource_global.h"

namespace resource
{
	/*
		* class DicomImageHeader
			- contains image's header information. (Volume header)
			- handled by class CW3Image3D
		* Purpose:
			- maintains header information as string pair. (to display on 2D-view)
			- handle all Dicom header information
			- use when Dicom exporting
	*/
	class RESOURCE_EXPORT DicomImageHeader
	{
	public:
		//DicomImageHeader(void);
		explicit DicomImageHeader(std::map<std::string, std::string>& list) { core_list_ = list; }
		~DicomImageHeader(void) {};

		DicomImageHeader(const DicomImageHeader&) = delete;

	public:
		// public functions.
		inline const std::map<std::string, std::string>& core_list(void) const
		{
			return core_list_;
		}

		QString GetStudyID();
		QString GetSeriesID();
		QString GetPatientID();
		QString GetPatientName();
		QString GetPatientBirthDate();
		QString GetPatientAge();
		QString GetPatientSex();
		QString GetStudyDate();
		QString GetStudyTime();
		QString GetSeriesDate();
		QString GetSeriesTime();
		QString GetDescription();
		QString GetKVP();
		QString GetModality();
		QString GetXRayTubeCurrent();

	private:
		QString FindHeaderValue(const QString& tag);

	private:
		// private member fields.
		std::map<std::string, std::string> core_list_;
	};
} // end of namespace resource
