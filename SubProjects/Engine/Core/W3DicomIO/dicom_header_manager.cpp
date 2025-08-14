#include "dicom_header_manager.h"

#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcuid.h"

#include "../../Common/Common/W3Memory.h"

DicomHeaderManager::DicomHeaderManager()
{

}

DicomHeaderManager::~DicomHeaderManager()
{
	if (cur_data_object_)
	{
		cur_data_object_->clear();
	}
}

DicomHeaderManager* DicomHeaderManager::GetInstance()
{
	static DicomHeaderManager instance;
	return &instance;
}

void DicomHeaderManager::SetDcmDataset(DcmDataset* input_data)
{
	if (input_data == nullptr)
	{
		return;
	}

	OFString str;
	input_data->findAndGetOFString(DCM_StudyInstanceUID, str);

	if (str.compare(key_) == 0)
	{
		return;
	}
	
	key_ = str;
	if (cur_data_object_)
	{
		cur_data_object_->clear();
	}

	cur_data_object_ = input_data->clone();
}
