#pragma once
/*=========================================================================
File:			dicom_header_manager.h
Language:		C++11
Library:		DCMTK 3.6.4, Standard C++ Library
Author:			LIM TAE KYUN
First Date:		2021-07-02
Last Modify:	2021-07-02

Copyright (c) 2020 All rights reserved by HDXWILL.
=========================================================================*/

#include "dcmtk/ofstd/ofstring.h"

#include "w3dicomio_global.h"

class DcmDataset;
class DcmObject;
class W3DICOMIO_EXPORT DicomHeaderManager
{
public:
	static DicomHeaderManager* GetInstance();
	
	void SetDcmDataset(DcmDataset* input_data);	

	DicomHeaderManager(const DicomHeaderManager&) = delete;
	const DicomHeaderManager& operator = (const DicomHeaderManager&) = delete;	

	inline DcmObject* cur_data_object() { return cur_data_object_; }

private:
	DicomHeaderManager();
	~DicomHeaderManager();

private:
	DcmObject* cur_data_object_ = nullptr;
	OFString key_;
};
