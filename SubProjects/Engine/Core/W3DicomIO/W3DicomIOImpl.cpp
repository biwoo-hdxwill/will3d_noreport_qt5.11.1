#include "W3DicomIOImpl.h"

#include <qbuffer.h>
#include <qfile.h>
#include <QDebug>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QSettings>
#include <QElapsedTimer>

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif

#include "dcmtk/dcmdata/dcitem.h"

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/common.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ImageHeader.h"

#include "../../Core/ImageProcessing/W3Classification.h"
#include "../../Core/ImageProcessing/W3DetectSliceLoc.h"

#include "W3DicomIOException.h"
#include "dicom_header_manager.h"
#include "lossless_decoder.h"

#if defined(__APPLE__)
typedef unsigned short UINT16, *PUINT16;
#endif

#define APPLY_SLOPE_TO_HISTOGRAM 1

namespace
{
	const char* kUnicode = "ISO_IR 192";
	//const char* kUnicode = "192";
	const float kTissueHU = 40.0f;
	const float kBoneHU = 1000.0f;
	const float kTheethHU = 3000.0f;

	const int kPow2_16 = static_cast<int>(pow(2, 16));
}  // end of namespace

char CW3DicomIOImpl::series_instance_uid_[100] = { '\0' };

CW3DicomIOImpl::CW3DicomIOImpl(void) { m_bProcessingAreaInputed = false; }

CW3DicomIOImpl::~CW3DicomIOImpl(void) {}

/*****************************
Interfaces for Reading.
******************************/
const bool CW3DicomIOImpl::setFiles(const std::vector<std::string> &fileNames)
{
	if (fileNames.size() <= 1)  // warning
	{
		common::Logger::instance()->Print(common::LogType::WRN,
			"Insufficient Dicom File Size");
		return false;
	}

	m_strFileNames = fileNames;
	m_nSeriesNum = 1;  // FIX TO SINGLE SERIES.

	DcmFileFormat fileFormat;
	OFCondition status = fileFormat.loadFile(fileNames.at(0).c_str());
	if (isStatusBad(status, "Non-Dicom File Format.")) return false;

	DicomImage *pImage = new DicomImage(m_strFileNames.at(0).c_str());

	unsigned int ndim = pImage->getFrameCount();

	if ((fileNames.size() == 1) && (ndim != 1))
	{
		m_eSeriesFileType.push_back(EFILE_TYPE::MULTI);
	}
	else
	{
		m_eSeriesFileType.push_back(EFILE_TYPE::SINGLE);
	}

	SAFE_DELETE_OBJECT(pImage);
	return true;
}

const std::vector<std::string> &CW3DicomIOImpl::GetDICOMFiles()
{
	return m_strFileNames;
}

void CW3DicomIOImpl::setArea(QRect Area)
{
	m_bProcessingAreaInputed = true;
	m_ProcessingArea = Area;
}

const bool CW3DicomIOImpl::setDirectory(const std::string &dirPath)
{
	return true;
}

/* Get # of series in files | directory. */
const unsigned int CW3DicomIOImpl::getNumberOfSeries(void) { return 0; }

/*
Get header information for (index)th volume among series.
*/
QString CW3DicomIOImpl::GetDicomInfo(DcmDataset* ds, DcmTagKey key)
{
	OFString str;
	ds->findAndGetOFString(key, str);

	if (is_unicode_)
	{
		QTextCodec* codec = QTextCodec::codecForName("UTF-8");
		return codec->toUnicode(str.c_str());
	}

	//return is_unicode_ ? QString::fromUtf8(str.c_str()) : QString::fromLocal8Bit(str.c_str());
	//return QString::fromUtf8(str.c_str());
	return QString::fromLocal8Bit(str.c_str());
}

QString CW3DicomIOImpl::GetDicomInfo(DcmDataset* ds, DcmTagKey key, unsigned int pos, QString& title)
{
	OFString str;
	ds->findAndGetOFString(key, str, pos);

	if (is_unicode_)
	{
		QTextCodec* codec = QTextCodec::codecForName("UTF-8");
		return codec->toUnicode(str.c_str());
	}

	//return is_unicode_ ? QString::fromUtf8(str.c_str()) : QString::fromLocal8Bit(str.c_str());
	//return QString::fromUtf8(str.c_str());
	return QString::fromLocal8Bit(str.c_str());
}

DcmElement* searchElement(DcmObject* _object, unsigned int _group, unsigned int _element)
{
	try
	{
		if (_object == NULL)
		{
			//throw DicomLibException("No data!", __FILE__, __LINE__);
		}

		DcmTagKey searchKey;
		searchKey.set(_group, _element);

		DcmStack stack;

		_object->search(searchKey, stack, ESM_fromHere, OFTrue);

		DcmElement* element = NULL;
		element = reinterpret_cast<DcmElement*>(stack.elem(0));

		return element;
	}
	catch (...)
	{
		throw;
	}
}


const bool CW3DicomIOImpl::getHeaderInfoFromFile(const std::string &filePath,
	HeaderList &listHeader,
	bool bSimple)
{
	listHeader.clear();

	DcmFileFormat fileFormat;

	OFCondition status = fileFormat.loadFile(filePath.c_str());
	if (!status.good()) return false;

	DcmDataset *pDS = fileFormat.getDataset();

	is_unicode_ = false;
	OFString character_set;
	if (pDS->findAndGetOFStringArray(DCM_SpecificCharacterSet, character_set).good())
	{
		QString q_character_set = character_set.c_str();
		if (q_character_set.contains(kUnicode))
		{
			is_unicode_ = true;
			//OFCondition convert_status = pDS->convertToUTF8();
			//is_unicode_ = convert_status.good();
		}
	}

	DcmFileFormat ff;
	ff.loadFile(filePath.c_str());
	DcmMetaInfo *m = ff.getMetaInfo();

	DcmElement* elem = NULL;
	elem = ::searchElement(m, 0002, 0002);

	listHeader["ImageOrientationPatient"] = GetDicomInfo(pDS, DCM_ImageOrientationPatient);
	listHeader["ImageOrientationSlide"] = GetDicomInfo(pDS, DCM_ImageOrientationSlide);
	listHeader["ImageOrientationVolume"] = GetDicomInfo(pDS, DCM_ImageOrientationVolume);
	listHeader["NumberOfFrames"] = GetDicomInfo(pDS, DCM_NumberOfFrames);


	listHeader["PatientID"] = GetDicomInfo(pDS, DCM_PatientID);
	listHeader["PatientName"] = GetDicomInfo(pDS, DCM_PatientName);

#if 0
	//
	DcmElement* element = nullptr;
	OFCondition condition = pDS->convertCharacterSet("ISO_IR 192");
	pDS->findAndGetElement(DCM_PatientName, element);
	unsigned long name_length = element->getLength();
	unsigned long name_length_field = element->getLengthField();
	Uint8* value = new Uint8[name_length];
	memset(value, 0, name_length);
	for (int i = 0; i < name_length; ++i)
	{
		OFCondition condition = element->getUint8(value[i], i);
		condition.good();
	}
	char* name = (char *)listHeader["PatientName"].toStdString().c_str();
	qDebug() << listHeader["PatientName"];
	qDebug() << listHeader["PatientName"].toStdString().c_str();
	qDebug() << name;
	for (int i = 0; i < listHeader["PatientName"].length(); ++i)
	{
		qDebug() << name[i];
	}
	//
#endif

	listHeader["DerivationDescription"] = GetDicomInfo(pDS, DCM_DerivationDescription);
	listHeader["PatientSex"] = GetDicomInfo(pDS, DCM_PatientSex);
	listHeader["PatientAge"] = GetDicomInfo(pDS, DCM_PatientAge);
	listHeader["PatientBirthDate"] = GetDicomInfo(pDS, DCM_PatientBirthDate);
	listHeader["StudyInstanceUID"] = GetDicomInfo(pDS, DCM_StudyInstanceUID);
	listHeader["SeriesInstanceUID"] = GetDicomInfo(pDS, DCM_SeriesInstanceUID);
	listHeader["StudyDate"] = GetDicomInfo(pDS, DCM_StudyDate);
	listHeader["SeriesDate"] = GetDicomInfo(pDS, DCM_SeriesDate);
	listHeader["StudyTime"] = GetDicomInfo(pDS, DCM_StudyTime);
	listHeader["SeriesTime"] = GetDicomInfo(pDS, DCM_SeriesTime);
	listHeader["Modality"] = GetDicomInfo(pDS, DCM_Modality);
	listHeader["KVP"] = GetDicomInfo(pDS, DCM_KVP);
	listHeader["XRayTubeCurrent"] = GetDicomInfo(pDS, DCM_XRayTubeCurrent);
	listHeader["ExposureTime"] = GetDicomInfo(pDS, DCM_ExposureTime);
	listHeader["PixelSpacing"] = GetDicomInfo(pDS, DCM_PixelSpacing);
	listHeader["SliceThickness"] = GetDicomInfo(pDS, DCM_SliceThickness);
	listHeader["BitsAllocated"] = GetDicomInfo(pDS, DCM_BitsAllocated);
	listHeader["BitsStored"] = GetDicomInfo(pDS, DCM_BitsStored);
	listHeader["HighBit"] = GetDicomInfo(pDS, DCM_HighBit);
	listHeader["PixelRepresentation"] = GetDicomInfo(pDS, DCM_PixelRepresentation);
	listHeader["RescaleIntercept"] = GetDicomInfo(pDS, DCM_RescaleIntercept);
	listHeader["RescaleSlope"] = GetDicomInfo(pDS, DCM_RescaleSlope);
	listHeader["Rows"] = GetDicomInfo(pDS, DCM_Rows);
	listHeader["Columns"] = GetDicomInfo(pDS, DCM_Columns);
	listHeader["WindowCenter"] = GetDicomInfo(pDS, DCM_WindowCenter);
	listHeader["WindowWidth"] = GetDicomInfo(pDS, DCM_WindowWidth);
	listHeader["StudyDescription"] = GetDicomInfo(pDS, DCM_StudyDescription);
	listHeader["SeriesDescription"] = GetDicomInfo(pDS, DCM_SeriesDescription);
	listHeader["SeriesNumber"] = GetDicomInfo(pDS, DCM_SeriesNumber);
#if 0
	listHeader["ImagePositionPatient"] =
		GetDicomInfo(pDS, DCM_ImagePositionPatient, 0, QString("ImagePositionPatient")) +
		"\\" +
		GetDicomInfo(pDS, DCM_ImagePositionPatient, 1, QString("ImagePositionPatient")) +
		"\\" +
		GetDicomInfo(pDS, DCM_ImagePositionPatient, 2, QString("ImagePositionPatient"));
#else
	listHeader["ImagePositionPatient"] = GetDicomInfo(pDS, DCM_ImagePositionPatient, 2, QString("ImagePositionPatient"));
#endif
	listHeader["InstanceNumber"] = GetDicomInfo(pDS, DCM_InstanceNumber);
	listHeader["SliceLocation"] = GetDicomInfo(pDS, DCM_SliceLocation);

	if (!bSimple)
	{
		listHeader["FileMetaInformationGroupLength"] =
			GetDicomInfo(pDS, DCM_FileMetaInformationGroupLength);
		listHeader["FileMetaInformationVersion"] =
			GetDicomInfo(pDS, DCM_FileMetaInformationVersion);
		listHeader["MediaStorageSOPClassUID"] =
			GetDicomInfo(pDS, DCM_MediaStorageSOPClassUID);
		listHeader["MediaStorageSOPInstanceUID"] =
			GetDicomInfo(pDS, DCM_MediaStorageSOPInstanceUID);
		listHeader["TransferSyntaxUID"] = GetDicomInfo(pDS, DCM_TransferSyntaxUID);
		listHeader["ImplementationClassUID"] =
			GetDicomInfo(pDS, DCM_ImplementationClassUID);
		listHeader["ImplementationVersionName"] =
			GetDicomInfo(pDS, DCM_ImplementationVersionName);
		listHeader["SpecificCharacterSet"] =
			GetDicomInfo(pDS, DCM_SpecificCharacterSet);
		listHeader["ImageType"] = GetDicomInfo(pDS, DCM_ImageType);
		listHeader["SOPClassUID"] = GetDicomInfo(pDS, DCM_SOPClassUID);
		listHeader["SOPInstanceUID"] = GetDicomInfo(pDS, DCM_SOPInstanceUID);
		listHeader["AcquisitionDate"] = GetDicomInfo(pDS, DCM_AcquisitionDate);
		listHeader["ContentDate"] = GetDicomInfo(pDS, DCM_ContentDate);
		listHeader["AcquisitionTime"] = GetDicomInfo(pDS, DCM_AcquisitionTime);
		listHeader["ContentTime"] = GetDicomInfo(pDS, DCM_ContentTime);
		listHeader["AccessionNumber"] = GetDicomInfo(pDS, DCM_AccessionNumber);
		listHeader["Manufacturer"] = GetDicomInfo(pDS, DCM_Manufacturer);
		listHeader["InstitutionName"] = GetDicomInfo(pDS, DCM_InstitutionName);
		listHeader["ManufacturerModelName"] =
			GetDicomInfo(pDS, DCM_ManufacturerModelName);
		listHeader["DerivationDescription"] =
			GetDicomInfo(pDS, DCM_DerivationDescription);
		listHeader["DerivationCodeSequence"] =
			GetDicomInfo(pDS, DCM_DerivationCodeSequence);
		listHeader["CodeValue"] = GetDicomInfo(pDS, DCM_CodeValue);
		listHeader["CodingSchemeDesignator"] =
			GetDicomInfo(pDS, DCM_CodingSchemeDesignator);
		listHeader["CodeMeaning"] = GetDicomInfo(pDS, DCM_CodeMeaning);
		listHeader["DistanceSourceToDetector"] =
			GetDicomInfo(pDS, DCM_DistanceSourceToDetector);
		listHeader["DistanceSourceToPatient"] =
			GetDicomInfo(pDS, DCM_DistanceSourceToPatient);
		listHeader["FieldOfViewDimensions"] =
			GetDicomInfo(pDS, DCM_FieldOfViewDimensions);
		listHeader["ImageOrientationPatient"] =
			GetDicomInfo(pDS, DCM_ImageOrientationPatient);
		listHeader["SamplesPerPixel"] = GetDicomInfo(pDS, DCM_SamplesPerPixel);
		listHeader["PhotometricInterpretation"] =
			GetDicomInfo(pDS, DCM_PhotometricInterpretation);
		listHeader["PixelData"] = GetDicomInfo(pDS, DCM_PixelData);
	}
	return true;
}

const bool CW3DicomIOImpl::getImageBufFromFile(const std::string& filePath,
	short *pImageBuf)
{
	DJDecoderRegistration::registerCodecs();

	DcmFileFormat fileFormat;
	OFCondition status = fileFormat.loadFile(filePath.c_str());
	if (isStatusBad(status, "Non-Dicom File Format.")) return false;

	DcmDataset *pDS = fileFormat.getDataset();
	DcmElement *element = nullptr;
	status = pDS->findAndGetElement(DCM_PixelData, element);
	if (isStatusBad(status, "Non-Dicom File Format.")) return false;

	UINT16 width = 0, height = 0;
	pDS->findAndGetUint16(DCM_Columns, width);
	pDS->findAndGetUint16(DCM_Rows, height);

	uchar* lossless_pixel_data = nullptr;
#if 0
	DicomImage dicom_image(filePath.c_str());
	if (dicom_image.getStatus() != EIS_Normal)
	{
#if 1
		QString path = "E:/DICOM/20180105140634397_0040.dcm";
		//QString path = QString::fromLocal8Bit(filePath.c_str());
		lossless_pixel_data = new uchar[width * height];
		LosslessDecoder decoder;
		bool result = decoder.DecodeGrayscale(path, lossless_pixel_data);
		QImage img(lossless_pixel_data, width, height, QImage::Format_Grayscale8);
		img.save(path + ".png", "PNG");
#else
		lossless_pixel_data = new uchar[width * height * 4];
		LosslessDecoder decoder;
		bool result = decoder.Decode(QString::fromLocal8Bit(filePath.c_str()), lossless_pixel_data);
#endif
	}
#endif

	DcmPixelData *pPix = OFstatic_cast(DcmPixelData *, element);
	UINT16 *pixData = nullptr;
	status = pPix->getUint16Array(pixData);
	if (pixData == nullptr)
	{
		Uint32 framesize = 0;
		element->getUncompressedFrameSize(pDS, framesize);
		short *buf = new short[framesize];
		OFString decompressedColorModel;
		Uint32 startFragment = 0;
		element->getUncompressedFrame(pDS, 0, startFragment, buf, framesize, decompressedColorModel);
		pDS->putAndInsertUint16Array(DCM_PixelData, (const Uint16 *)buf, framesize);
		pDS->removeAllButCurrentRepresentations();
		status = pDS->findAndGetElement(DCM_PixelData, element);
		if (isStatusBad(status, "Non-Dicom File Format.")) return false;

		DcmPixelData *pPix = OFstatic_cast(DcmPixelData *, element);
		status = pPix->getUint16Array(pixData);
		SAFE_DELETE_OBJECT(buf);
	}

	if (isStatusBad(status, "Non-Dicom File Format.") || pixData == nullptr)
	{
		return false;
	}

	int dPixelRepresentation = 0;
	float slope = 0.0f, fInterCept = 0.0f;

	OFString Temp;
	pDS->findAndGetOFString(DCM_RescaleSlope, Temp);
	slope = atof(Temp.c_str());
	pDS->findAndGetOFString(DCM_RescaleIntercept, Temp);
	fInterCept = atof(Temp.c_str());
	pDS->findAndGetOFString(DCM_PixelRepresentation, Temp);
	dPixelRepresentation = atoi(Temp.c_str());

	if (slope < 0.001f) slope = 1.0f;

	if (lossless_pixel_data)
	{
		for (int i = 0; i < width * height; ++i)
		{
			short casted_value = static_cast<short>(static_cast<double>(lossless_pixel_data[i]) / 255.0 * 32767.0);
			pImageBuf[i] = (slope * casted_value) + fInterCept;
		}
	}
	else
	{
		for (int i = 0; i < width * height; ++i)
		{
			pImageBuf[i] = (slope * pixData[i]) + fInterCept;
		}
	}

	DJDecoderRegistration::cleanup();

	return true;
}

/*
Get volume data for (index)th volume among series.
*/
const bool CW3DicomIOImpl::readVolume(const int index, CW3Image3D *&vol)
{
	QElapsedTimer timer;
	timer.start();
	common::Logger::instance()->Print(common::LogType::INF, "start readVolume");

	// Initialize JPEG decoders
	DJDecoderRegistration::registerCodecs();
	if (m_eSeriesFileType.empty()) return false;

	if (vol != nullptr)
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			"Volume already has memory.");
		throw CW3DicomIOException(CW3DicomIOException::EID::INVALID_VOLUME,
			"Volume already has memory.");
	}

	if (m_eSeriesFileType.at(0) == EFILE_TYPE::SINGLE)  // read single-frame dicom files.
	{
		DcmFileFormat fileFormat;
		OFCondition status = fileFormat.loadFile(m_strFileNames.at(0).c_str());
		if (isStatusBad(status, "Non-Dicom File Format."))
		{
			return false;
		}

		DcmDataset *pDS = fileFormat.getDataset();
		DicomHeaderManager::GetInstance()->SetDcmDataset(pDS);

		int dPixelRepresentation = 0;
		unsigned long win_center = 0, win_width = 0;
		float fPixelSpacing, slope, fInterCept, fSliceLocation, slice_thickness;
		fPixelSpacing = slope = fInterCept = fSliceLocation = slice_thickness = 0.0f;

		OFString Temp;
		// pDS->findAndGetOFString(DCM_SpacingBetweenSlices, Temp);	fSliceSpacing =
		// atof(Temp.c_str());//DCM_SpacingBetweenSlices
		pDS->findAndGetOFString(DCM_SliceLocation, Temp);
		fSliceLocation = atof(Temp.c_str());  // DCM_SliceLocation
		pDS->findAndGetOFString(DCM_SliceThickness, Temp);
		slice_thickness = atof(Temp.c_str());  // DCM_SliceThickness
		pDS->findAndGetOFString(DCM_PixelSpacing, Temp);
		fPixelSpacing = atof(Temp.c_str());  // DCM_PixelSpacing
		pDS->findAndGetOFString(DCM_RescaleSlope, Temp);
		slope = atof(Temp.c_str());  // DCM_RescaleSlope
		pDS->findAndGetOFString(DCM_RescaleIntercept, Temp);
		fInterCept = atof(Temp.c_str());  // DCM_RescaleIntercept
		pDS->findAndGetOFString(DCM_WindowCenter, Temp);
		win_center = static_cast<int>(atof(Temp.c_str()));  // DCM_WindowCenter
		pDS->findAndGetOFString(DCM_WindowWidth, Temp);
		win_width = static_cast<int>(atof(Temp.c_str()));  // DCM_WindowWidth
		pDS->findAndGetOFString(DCM_PixelRepresentation, Temp);
		dPixelRepresentation = atoi(Temp.c_str());

		if (dPixelRepresentation == 0)
		{
			DicomImage dicom_image(m_strFileNames.front().c_str());
			if (dicom_image.getStatus() != EIS_Normal)
			{
				common::Logger::instance()->Print(
					common::LogType::ERR, "failed to open dicom_image.");
				throw CW3DicomIOException(
					CW3DicomIOException::EID::INVALID_VOLUME,
					"failed to open image.");
			}

			double tmp_min = std::numeric_limits<double>::max();
			double tmp_max = std::numeric_limits<double>::min();
			int res = dicom_image.getMinMaxValues(tmp_min, tmp_max);

			if (static_cast<int>(tmp_min) < static_cast<int>(fInterCept))
			{
				dPixelRepresentation = 1;
			}
		}

		pDS->findAndGetOFString(DCM_BitsAllocated, Temp);
		int bits_allocated = atoi(Temp.c_str());
		pDS->findAndGetOFString(DCM_BitsStored, Temp);
		int bits_stored = atoi(Temp.c_str());
		common::Logger::instance()->Print(common::LogType::INF, "pixel representation :" + QString("%1").arg(dPixelRepresentation).toStdString());
		common::Logger::instance()->Print(common::LogType::INF, "bits allocated :" + QString("%1").arg(bits_allocated).toStdString());
		common::Logger::instance()->Print(common::LogType::INF, "bits stored :" + QString("%1").arg(bits_stored).toStdString());

		if (slope < 0.001f)
			slope = 1.0f;

		/* Setting common header information. */
		UINT16 width = 0, height = 0;
		pDS->findAndGetUint16(DCM_Columns, width);
		pDS->findAndGetUint16(DCM_Rows, height);

		int sx = 0, sy = 0;
		UINT16 oriWidth = 0, oriHeight = 0;
		if (m_bProcessingAreaInputed)
		{
			sx = m_ProcessingArea.left();
			sy = m_ProcessingArea.top();
			pDS->findAndGetUint16(DCM_Columns, oriWidth);
			pDS->findAndGetUint16(DCM_Rows, oriHeight);
			width = m_ProcessingArea.width();
			height = m_ProcessingArea.height();
		}

		// create volume & set information.
		unsigned int depth = m_strFileNames.size();
		DWORDLONG volSize = width * height * depth * sizeof(unsigned short);

		MEMORYSTATUSEX memory;
		ZeroMemory(&memory, sizeof(MEMORYSTATUSEX));
		memory.dwLength = sizeof(MEMORYSTATUSEX);
		if (GlobalMemoryStatusEx(&memory))
		{
			if (memory.ullAvailPhys < volSize)
			{
				QString error_msg = lang::LanguagePack::msg_90();
				common::Logger::instance()->Print(common::LogType::ERR, error_msg.toStdString());
				throw CW3DicomIOException(CW3DicomIOException::EID::INSUFFICIENT_MEMORY, error_msg.toStdString());
				CW3MessageBox msg_box("Will3D", error_msg, CW3MessageBox::Critical);
				msg_box.exec();
				return false;
			}
		}
		else
		{
			return false;
		}

		try
		{
			vol = new CW3Image3D(width, height, depth);
		}
		catch (std::bad_alloc)
		{
			common::Logger::instance()->Print(common::LogType::ERR,
				"Memory Allocation Failure.");
			throw CW3DicomIOException(CW3DicomIOException::EID::UNKNOWN,
				"Memory Allocation Failure.");
		}

		if (slice_thickness < 0.001f)
		{
			float fSliceLocation_0 = fSliceLocation;
			float fSliceLocation_1 = 0.0f;

			// for fSliceLocation_1
			DcmFileFormat fileFormat;
			OFCondition status = fileFormat.loadFile(m_strFileNames.at(1).c_str());
			if (isStatusBad(status, "Non-Dicom File Format.")) return false;

			DcmDataset *pDS = fileFormat.getDataset();
			OFString Temp;
			pDS->findAndGetOFString(DCM_SliceLocation, Temp);
			fSliceLocation_1 = atof(Temp.c_str());  // DCM_SliceLocation

			float new_slice_thickness = std::abs(fSliceLocation_1 - fSliceLocation_0);

			slice_thickness =
				(new_slice_thickness < 0.001f) ? fPixelSpacing : new_slice_thickness;
		}

		vol->setPixelSpacing(fPixelSpacing);
		vol->setSliceSpacing(slice_thickness);
		vol->setSlope(slope);
		vol->set_bits_stored(bits_stored);
		vol->set_pixel_representation(dPixelRepresentation);

#if 1
		vol->setIntercept(fInterCept);
#else
#if 1
		if (dPixelRepresentation)
		{
			int window_min = (int)((float)win_center - (float)win_width * 0.5f);
			vol->setIntercept(window_min);
		}
		else
		{
			vol->setIntercept(fInterCept);
		}
#else
		if (fInterCept == 0.0f && dPixelRepresentation)
		{
			vol->setIntercept(-1000);
		}
		else
		{
			vol->setIntercept(fInterCept);
		}
#endif
#endif

		vol->setWindowing(win_center - vol->intercept(), win_width);

		HeaderList hd;
		getHeaderInfoFromFile(m_strFileNames.at(0).c_str(), hd, false);
		std::shared_ptr<CW3ImageHeader> header(new CW3ImageHeader(hd));
		vol->setHeader(header);
		ResourceContainer::GetInstance()->SetDicomHeaderResource(header);

		Uint32 framesize = 0;
		int volDepth = vol->depth();

#if 0
		QElapsedTimer timer;
		timer.start();
		common::Logger::instance()->Print(common::LogType::INF, "start Read16BitSlices");

		if (dPixelRepresentation)
		{
			Read16BitSlices<short>(vol, m_strFileNames, sx, sy, width, height);
		}
		else
		{
			Read16BitSlices<unsigned short>(vol, m_strFileNames, sx, sy, width, height);
		}

		common::Logger::instance()->Print(common::LogType::INF, "end Read16BitSlices : " + QString::number(timer.elapsed()).toStdString() + " ms");
#else
		ReadPixelData(vol, sx, sy);
#endif

#if 0 // ct dicom에 jpg를 덧씌워서 rgb dicom 으로 저장 테스트
		int vol_depth = vol->depth();
		int vol_height = vol->height();
		int vol_width = vol->width();
		unsigned short min = vol->getMin();
		unsigned short max = vol->getMax();

		QFileInfo dcm_info(QString::fromStdString(m_strFileNames.at(0)));

#define SAVE_TO_U16 1

		for (int z = 0; z < vol_depth; ++z)
		{
			unsigned short* old_slice = vol->getData()[z];

			unsigned char* new_slice_rgb = nullptr;
			W3::p_allocate_1D(&new_slice_rgb, vol_width * vol_height * 3);
			memset(new_slice_rgb, 0, sizeof(unsigned char) * vol_width * vol_height * 3);

#if SAVE_TO_U16
			unsigned short* new_slice_gray = nullptr;
			W3::p_allocate_1D(&new_slice_gray, vol_width * vol_height);
			memset(new_slice_gray, 0, sizeof(unsigned short) * vol_width * vol_height);
#else
			unsigned char* new_slice_gray = nullptr;
			W3::p_allocate_1D(&new_slice_gray, vol_width * vol_height);
			memset(new_slice_gray, 0, sizeof(unsigned char) * vol_width * vol_height);
#endif
			QString implant_image_file_name;
			implant_image_file_name.sprintf("%04d.bmp", z);
			QImage implant_image(dcm_info.absolutePath() + "/" + implant_image_file_name);
			unsigned char* implant_image_bits = nullptr;
			if (!implant_image.isNull())
			{
				implant_image_bits = implant_image.bits();
			}

			for (int xy = 0; xy < vol_width * vol_height; ++xy)
			{
				unsigned char implant_image_value_r = 0;
				unsigned char implant_image_value_g = 0;
				unsigned char implant_image_value_b = 0;
				if (implant_image_bits)
				{
					implant_image_value_r = implant_image_bits[xy * 4];
					implant_image_value_g = implant_image_bits[xy * 4 + 1];
					implant_image_value_b = implant_image_bits[xy * 4 + 2];
				}

				float new_value_u8 = static_cast<float>(old_slice[xy] - min) / (max - min) * 255.0f;
				//float new_value_u16 = static_cast<float>(old_slice[xy] - min) / (max - min) * 65535.0f;
				unsigned short new_value_u16 = old_slice[xy];

				if (implant_image_value_r || implant_image_value_g || implant_image_value_b)
				{
					new_value_u8 = 255;
					new_value_u16 = max + 100;
					new_slice_rgb[xy * 3] = implant_image_value_r;
					new_slice_rgb[xy * 3 + 1] = implant_image_value_g;
					new_slice_rgb[xy * 3 + 2] = implant_image_value_b;
				}
				else
				{
					new_slice_rgb[xy * 3] = static_cast<unsigned char>(new_value_u8);
					new_slice_rgb[xy * 3 + 1] = static_cast<unsigned char>(new_value_u8);
					new_slice_rgb[xy * 3 + 2] = static_cast<unsigned char>(new_value_u8);
				}

#if SAVE_TO_U16
				new_slice_gray[xy] = static_cast<unsigned short>(new_value_u16);
#else
				new_slice_gray[xy] = static_cast<unsigned char>(new_value_u8);
#endif
			}
			HeaderList header;
			getHeaderInfoFromFile(m_strFileNames.at(z).c_str(), header, false);

#if SAVE_TO_U16
			//header["WindowCenter"] = "32767";
			//header["WindowWidth"] = "65536";
			header["WindowCenter"] = QString::number((vol->windowCenter() - vol->intercept()) * vol->slope());
			header["WindowWidth"] = QString::number((vol->windowWidth() - vol->intercept()) * vol->slope());
			header["BitsAllocated"] = "16";
			header["BitsStored"] = "16";
			header["HighBit"] = "15";
#else
			header["PlanarConfiguration"] = "0";
			header["PixelRepresentation"] = "0";
			header["WindowCenter"] = "127";
			header["WindowWidth"] = "256";
			header["BitsAllocated"] = "8";
			header["BitsStored"] = "8";
			header["HighBit"] = "7";
#endif
			header["SamplesPerPixel"] = "1";
			header["PhotometricInterpretation"] = "MONOCHROME2";
			header["RescaleSlope"] = "1";
			header["RescaleIntercept"] = "0";

			QString file_name_gray;
			file_name_gray.sprintf("C:/Users/HDXWill/Desktop/test_dcm/gray/%04d_gray.dcm", z);
			writeDicom(file_name_gray.toStdString(), header, new_slice_gray);

			header["PlanarConfiguration"] = "0";
			header["PixelRepresentation"] = "0";
			header["RescaleSlope"] = "1";
			header["RescaleIntercept"] = "0";
			header["WindowCenter"] = "127";
			header["WindowWidth"] = "256";
			header["BitsAllocated"] = "8";
			header["BitsStored"] = "8";
			header["HighBit"] = "7";
			header["SamplesPerPixel"] = "3";
			header["PhotometricInterpretation"] = "RGB";

			QString file_name_rgb;
			file_name_rgb.sprintf("C:/Users/HDXWill/Desktop/test_dcm/rgb/%04d_rgb.dcm", z);
			//writeDicom(file_name_rgb.toStdString(), header, new_slice_rgb);

			SAFE_DELETE_ARRAY(new_slice_rgb);
			SAFE_DELETE_ARRAY(new_slice_gray);
		}
#endif
		PostProcessing(vol);
	}

	DJDecoderRegistration::cleanup();

	common::Logger::instance()->Print(common::LogType::INF, "end readVolume : " + QString::number(timer.elapsed()).toStdString() + " ms");

	return true;
}

/*****************************
Interfaces for Writing.
(exporting Dicom files)
******************************/
/*
Export Dicom File(s).

dirPath		: Directory Path for Writing Dicom
listHeader	: Dicom Header Information
ppData		: Volume Data
type		: Specifies Dicom File Format. (single | multi frame)
*/
// template<typename TYPE>
const bool CW3DicomIOImpl::writeDicom(const std::string& dirPath, HeaderList listHeader, unsigned char* data)
{
	DcmFileFormat fileFormat;
	DcmDataset* dataset = fileFormat.getDataset();

	WriteHeader(dataset, listHeader);

	if (listHeader.find("BitsAllocated") != listHeader.end())
	{
		dataset->putAndInsertString(DCM_BitsAllocated, listHeader.at("BitsAllocated").toLocal8Bit().data());
	}
	if (listHeader.find("BitsStored") != listHeader.end())
	{
		dataset->putAndInsertString(DCM_BitsStored, listHeader.at("BitsStored").toLocal8Bit().data());
	}
	if (listHeader.find("HighBit") != listHeader.end())
	{
		dataset->putAndInsertString(DCM_HighBit, listHeader.at("HighBit").toLocal8Bit().data());
	}

	int samples_per_pexel = 1;
	if (listHeader.find("SamplesPerPixel") != listHeader.end())
	{
		samples_per_pexel = atoi(listHeader.at("SamplesPerPixel").toLocal8Bit().data());
		dataset->putAndInsertString(DCM_SamplesPerPixel, listHeader.at("SamplesPerPixel").toLocal8Bit().data());
	}
	if (listHeader.find("PhotometricInterpretation") != listHeader.end())
	{
		dataset->putAndInsertString(DCM_PhotometricInterpretation, listHeader.at("PhotometricInterpretation").toLocal8Bit().data());
	}

	int rows = 0;
	int columns = 0;
	if (listHeader.find("Rows") != listHeader.end())
	{
		rows = atoi(listHeader.at("Rows").toLocal8Bit().data());
		dataset->putAndInsertString(DCM_Rows, listHeader.at("Rows").toLocal8Bit().data());
	}
	if (listHeader.find("Columns") != listHeader.end())
	{
		columns = atoi(listHeader.at("Columns").toLocal8Bit().data());
		dataset->putAndInsertString(DCM_Columns, listHeader.at("Columns").toLocal8Bit().data());
	}
	if (columns * rows < 1)
	{
		return false;
	}

	dataset->putAndInsertUint8Array(DCM_PixelData, data, columns * rows * samples_per_pexel);

	DJEncoderRegistration::registerCodecs();
	DJ_RPLossless params;
	E_TransferSyntax filter = EXS_LittleEndianImplicit;
	dataset->chooseRepresentation(filter, &params);
	if (dataset->canWriteXfer(filter))
	{
		OFCondition status = fileFormat.saveFile(dirPath.c_str(), filter);
		if (status.bad())
		{
			common::Logger::instance()->Print(common::LogType::ERR, "ERROR writeDicom : " + dirPath);
			return false;
		}
		DJEncoderRegistration::cleanup();
	}

	return true;
}

const bool CW3DicomIOImpl::writeDicom(const std::string& dirPath, HeaderList listHeader, unsigned short* data)
{
	/*****************************************
	Export Dicom Series of Single-Frame
	**************************************/

	DcmFileFormat fileFormat;
	DcmDataset* dataset = fileFormat.getDataset();
	WriteHeader(dataset, listHeader);

	PutAndInsertString(dataset, listHeader, DCM_BitsAllocated, "BitsAllocated");
	PutAndInsertString(dataset, listHeader, DCM_BitsStored, "BitsStored");
	PutAndInsertString(dataset, listHeader, DCM_HighBit, "HighBit");

	int samples_per_pexel = 1;
	if (listHeader.find("SamplesPerPixel") != listHeader.end())
	{
		samples_per_pexel = atoi(listHeader.at("SamplesPerPixel").toLocal8Bit().data());
		dataset->putAndInsertString(DCM_SamplesPerPixel, listHeader.at("SamplesPerPixel").toLocal8Bit().data());
	}
	if (listHeader.find("PhotometricInterpretation") != listHeader.end())
	{
		dataset->putAndInsertString(DCM_PhotometricInterpretation, listHeader.at("PhotometricInterpretation").toLocal8Bit().data());
	}

	int rows = 0;
	int columns = 0;
	if (PutAndInsertString(dataset, listHeader, DCM_Rows, "Rows"))
	{
		rows = atoi(listHeader.at("Rows").toLocal8Bit().data());
	}
	if (PutAndInsertString(dataset, listHeader, DCM_Columns, "Columns"))
	{
		columns = atoi(listHeader.at("Columns").toLocal8Bit().data());
	}
	if (columns * rows < 1)
	{
		return false;
	}

#if 0
	OFCondition status = fileFormat.saveFile(dirPath.c_str(), EXS_LittleEndianImplicit);
	if (status.bad())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "ERROR writeDicom : " + dirPath);
	}
#else
	DJEncoderRegistration::registerCodecs();
	DJ_RPLossless params;

	E_TransferSyntax filter = EXS_LittleEndianImplicit;
	OFCondition condition = dataset->chooseRepresentation(filter, &params);

	dataset->putAndInsertUint16Array(DCM_PixelData, data, columns * rows * samples_per_pexel);
	if (dataset->canWriteXfer(filter))
	{
		OFCondition status = fileFormat.saveFile(dirPath.c_str(), filter);
		if (status.bad())
		{
			common::Logger::instance()->Print(common::LogType::ERR, "ERROR writeDicom : " + dirPath);
			return false;
		}
		DJEncoderRegistration::cleanup();
	}
#endif
	return true;
}

const bool CW3DicomIOImpl::WriteDicom(const std::string& dir_path, unsigned short* data, const int instance_number, const int rows, const int columns)
{
	DcmObject* cur_dcm_object = DicomHeaderManager::GetInstance()->cur_data_object();
	if (cur_dcm_object == nullptr)
	{
		return false;
	}

	DcmFileFormat fileFormat;
	DcmDataset* dataset = fileFormat.getDataset();
	dataset->copyFrom(*cur_dcm_object);

#if 0
	E_TransferSyntax filter = dataset->getCurrentXfer();
	dataset->putAndInsertUint16Array(DCM_PixelData, data, columns * rows * samples_per_pexel);
	OFCondition status = fileFormat.saveFile(dir_path.c_str(), filter);
	if (status.bad())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "ERROR writeDicom : " + dir_path);
		return false;
	}
#else
	DJEncoderRegistration::registerCodecs();
	DJDecoderRegistration::registerCodecs();
	DJ_RPLossless params;
	bool is_ok = true;

	E_TransferSyntax filter = EXS_LittleEndianImplicit;
	if (dataset->chooseRepresentation(filter, &params).good())
	{
		char uid[100];
		dataset->putAndInsertString(DCM_SOPInstanceUID, dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT));
		dataset->putAndInsertString(DCM_SeriesInstanceUID, series_instance_uid_);

		QString instance_num = QString::number(instance_number);
		dataset->putAndInsertString(DCM_InstanceNumber, instance_num.toLocal8Bit().data());

		QString rows_str = QString::number(rows);
		dataset->putAndInsertString(DCM_Rows, rows_str.toLocal8Bit().data());

		QString columns_str = QString::number(columns);
		dataset->putAndInsertString(DCM_Columns, columns_str.toLocal8Bit().data());

		QString seris_number = GlobalPreferences::GetInstance()->GetPACSSerisNumber();
		dataset->putAndInsertString(DCM_SeriesNumber, seris_number.toStdString().c_str());

		QDate current_date = QDate::currentDate();
		QString current_date_str = current_date.toString("yyyyMMdd");
		dataset->putAndInsertString(DCM_SeriesDate, current_date_str.toStdString().c_str());

		OFString spp;
		dataset->findAndGetOFString(DCM_SamplesPerPixel, spp);
		int samples_per_pexel = static_cast<int>(atof(spp.c_str()));

		dataset->putAndInsertUint16Array(DCM_PixelData, data, columns * rows * samples_per_pexel);
		if (dataset->canWriteXfer(filter))
		{
			if (fileFormat.saveFile(dir_path.c_str(), filter).bad())
			{
				is_ok = false;
			}
		}
	}
	else
	{
		is_ok = false;
	}
	DJEncoderRegistration::cleanup();
	DJDecoderRegistration::cleanup();
#endif
	if (!is_ok)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "ERROR writeDicom : " + dir_path);
	}

	return is_ok;
}

const bool CW3DicomIOImpl::WriteDicomRGB(const std::string& dir_path, unsigned char* data, const int instance_number, const int rows, const int columns)
{
	DcmObject* cur_dcm_object = DicomHeaderManager::GetInstance()->cur_data_object();
	if (cur_dcm_object == nullptr)
	{
		return false;
	}

	DcmFileFormat fileFormat;
	DcmDataset* dataset = fileFormat.getDataset();
	dataset->copyFrom(*cur_dcm_object);

#if 0
	//E_TransferSyntax filter = dataset->getCurrentXfer();
	dataset->putAndInsertUint8Array(DCM_PixelData, data, columns * rows * samples_per_pexel);
	OFCondition status = fileFormat.saveFile(dir_path.c_str(), EXS_LittleEndianImplicit);
	if (status.bad())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "ERROR writeDicom : " + dir_path);
		return false;
	}
#else
	DJEncoderRegistration::registerCodecs();
	DJDecoderRegistration::registerCodecs();
	DJ_RPLossless params;

	bool is_ok = true;

	E_TransferSyntax filter = EXS_LittleEndianImplicit;
	if (dataset->chooseRepresentation(filter, &params).good())
	{
		char uid[100];
		dataset->putAndInsertString(DCM_SOPInstanceUID, dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT));
		dataset->putAndInsertString(DCM_SeriesInstanceUID, series_instance_uid_);

		QString instance_num = QString::number(instance_number);
		dataset->putAndInsertString(DCM_InstanceNumber, instance_num.toLocal8Bit().data());

		QString rows_str = QString::number(rows);
		dataset->putAndInsertString(DCM_Rows, rows_str.toLocal8Bit().data());

		QString columns_str = QString::number(columns);
		dataset->putAndInsertString(DCM_Columns, columns_str.toLocal8Bit().data());

		QString seris_number = GlobalPreferences::GetInstance()->GetPACSSerisNumber();
		dataset->putAndInsertString(DCM_SeriesNumber, seris_number.toStdString().c_str());

		QDate current_date = QDate::currentDate();
		QString current_date_str = current_date.toString("yyyyMMdd");
		dataset->putAndInsertString(DCM_SeriesDate, current_date_str.toStdString().c_str());

		dataset->putAndInsertUint16(DCM_PixelRepresentation, 0);
		dataset->putAndInsertString(DCM_PhotometricInterpretation, "RGB");
		dataset->putAndInsertUint16(DCM_PlanarConfiguration, 0);
		dataset->putAndInsertUint16(DCM_BitsAllocated, 8);
		dataset->putAndInsertUint16(DCM_BitsStored, 8);
		dataset->putAndInsertUint16(DCM_HighBit, 7);
		dataset->putAndInsertUint16(DCM_SamplesPerPixel, 3);
		int samples_per_pexel = 3;

		dataset->putAndInsertString(DCM_WindowCenter, "128");
		dataset->putAndInsertString(DCM_WindowWidth, "256");

		dataset->putAndInsertUint8Array(DCM_PixelData, data, columns * rows * samples_per_pexel);
		if (dataset->canWriteXfer(filter))
		{
			if (fileFormat.saveFile(dir_path.c_str(), filter).bad())
			{
				is_ok = false;
			}
		}
	}
	else
	{
		is_ok = false;
	}
	DJEncoderRegistration::cleanup();
	DJDecoderRegistration::cleanup();
#endif
	if (!is_ok)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "ERROR writeDicom : " + dir_path);
	}

	return is_ok;
}

void CW3DicomIOImpl::WriteHeader(DcmDataset*& dataset, HeaderList& header)
{
	PutAndInsertString(dataset, header, DCM_SOPClassUID, "SOPClassUID");

	char uid[100];
	dataset->putAndInsertString(DCM_SOPInstanceUID, dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT));

	PutAndInsertString(dataset, header, DCM_ImageType, "ImageType");
	PutAndInsertString(dataset, header, DCM_StudyInstanceUID, "StudyInstanceUID");
	PutAndInsertString(dataset, header, DCM_SeriesInstanceUID, "SeriesInstanceUID");
	PutAndInsertString(dataset, header, DCM_StudyDate, "StudyDate");
	PutAndInsertString(dataset, header, DCM_SeriesDate, "SeriesDate");
	PutAndInsertString(dataset, header, DCM_StudyTime, "StudyTime");
	PutAndInsertString(dataset, header, DCM_SeriesTime, "SeriesTime");
	PutAndInsertString(dataset, header, DCM_Modality, "Modality");
	PutAndInsertString(dataset, header, DCM_Manufacturer, "Manufacturer");
	PutAndInsertString(dataset, header, DCM_StudyDescription, "StudyDescription");
	PutAndInsertString(dataset, header, DCM_DerivationDescription, "DerivationDescription");
	PutAndInsertString(dataset, header, DCM_PatientName, "PatientName");
	PutAndInsertString(dataset, header, DCM_PatientID, "PatientID");
	PutAndInsertString(dataset, header, DCM_PatientBirthDate, "PatientBirthDate");
	PutAndInsertString(dataset, header, DCM_PatientSex, "PatientSex");
	PutAndInsertString(dataset, header, DCM_PatientSex, "PatientSex");
	PutAndInsertString(dataset, header, DCM_SliceThickness, "SliceThickness");
	PutAndInsertString(dataset, header, DCM_SliceLocation, "SliceLocation");
	PutAndInsertString(dataset, header, DCM_SeriesNumber, "SeriesNumber");
	PutAndInsertString(dataset, header, DCM_InstanceNumber, "InstanceNumber");
	PutAndInsertString(dataset, header, DCM_NumberOfFrames, "NumberOfFrames");
	PutAndInsertString(dataset, header, DCM_PlanarConfiguration, "PlanarConfiguration");
	PutAndInsertString(dataset, header, DCM_PixelRepresentation, "PixelRepresentation");
	PutAndInsertString(dataset, header, DCM_WindowCenter, "WindowCenter");
	PutAndInsertString(dataset, header, DCM_WindowWidth, "WindowWidth");
	PutAndInsertString(dataset, header, DCM_RescaleSlope, "RescaleSlope");
	PutAndInsertString(dataset, header, DCM_RescaleIntercept, "RescaleIntercept");

	if (header.find("PixelSpacing") != header.end())
	{
		std::string pixel_spacing = header.at("PixelSpacing").toLocal8Bit().toStdString();
		pixel_spacing = pixel_spacing + "\\" + pixel_spacing;
		dataset->putAndInsertString(DCM_PixelSpacing, pixel_spacing.c_str());
	}
	if (header.find("PixelAspectRatio") != header.end())
	{
		std::string pixel_aspect_ratio = header.at("PixelAspectRatio").toLocal8Bit().toStdString();
		pixel_aspect_ratio = pixel_aspect_ratio + "\\" + pixel_aspect_ratio;
		dataset->putAndInsertString(DCM_PixelAspectRatio, pixel_aspect_ratio.c_str());
	}
	if (header.find("ImagePositionPatient") != header.end())
	{
		std::string image_position_patient = "0.0\\0.0\\" + header.at("ImagePositionPatient").toLocal8Bit().toStdString();
		dataset->putAndInsertString(DCM_ImagePositionPatient, image_position_patient.c_str());
	}
	if (header.find("ImageOrientationPatient") != header.end())
	{
		std::string image_position_patient = "0.0\\0.0\\" + header.at("ImagePositionPatient").toLocal8Bit().toStdString();
		dataset->putAndInsertString(DCM_ImageOrientationPatient, image_position_patient.c_str());
	}
}

std::vector<std::string> CW3DicomIOImpl::getHeader(HeaderList &hd,
	CW3Image3D *&vol)
{
	std::vector<std::string> list;
	for (auto &i : hd)
	{
		if (m_bProcessingAreaInputed)
		{
			if (i.first == std::string("Rows"))
				i.second = QString::number(vol->height());

			if (i.first == std::string("Columns"))
				i.second = QString::number(vol->width());
		}

		if (i.first == std::string("PatientName"))
			if (i.first == std::string("PatientID"))
				if (i.first == std::string("PatientBirthDate"))
					if (i.first == std::string("PatientSex"))
					{
						std::string str(i.first + " : " + i.second.toStdString());
						list.push_back(str);
					}
	}
	return list;
}
const bool CW3DicomIOImpl::isStatusBad(const OFCondition &status,
	const char *errMsg)
{
	if (status.bad())
	{
		common::Logger::instance()->Print(common::LogType::ERR, errMsg);
		//throw CW3DicomIOException(CW3DicomIOException::EID::INVALID_FILE_FORMAT, errMsg);
		return true;
	}
	return false;
}

const bool CW3DicomIOImpl::CheckFile(const std::string& file_path)
{
	DcmFileFormat dcm_file_format;
	OFCondition status = dcm_file_format.loadFile(file_path.c_str());
	if (isStatusBad(status, "Non-Dicom File Format."))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CW3DicomIOImpl::CreateSeriesInstanceUID()
{
	if (series_instance_uid_[0] != '\0')
	{
		return;
	}

	char uid[100];
	dcmGenerateUniqueIdentifier(uid, SITE_SERIES_UID_ROOT);
	memcpy(series_instance_uid_, uid, sizeof(char) * 100);
}

void CW3DicomIOImpl::InitSeriesInstanceUID()
{
	memset(series_instance_uid_, 0, sizeof(char) * 100);
	series_instance_uid_[0] = '\0';
}

void CW3DicomIOImpl::PostProcessing(CW3Image3D *&vol)
{
	QElapsedTimer timer;
	timer.start();
	common::Logger::instance()->Print(common::LogType::INF, "start PostProcessing");

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	bool otf_auto_mode = settings.value("OTF/auto", true).toBool();
	int otf_additional_offset = settings.value("OTF/offset", 0).toInt();

	int air_threshold = 0, tissue_threshold = 0, bone_threshold = 0;
	SliceLoc slice_loc;
	float slope = vol->slope();
	int intercept = static_cast<int>(vol->intercept());

	CW3Classification::ThresholderVolume(*vol, &air_threshold, &tissue_threshold, &bone_threshold);

#if 0
	if (otf_auto_mode)
	{
#if 1
		air_threshold *= slope;
		tissue_threshold *= slope;
		bone_threshold *= slope;
#endif
	}
	else
	{
		air_threshold = kTissueHU - intercept;
		tissue_threshold = kBoneHU - intercept;
		bone_threshold = kTheethHU - intercept;
	}
#else
#if APPLY_SLOPE_TO_HISTOGRAM
	if (!otf_auto_mode)
	{
		tissue_threshold = kBoneHU - intercept;
	}
#else
	air_threshold *= slope;
	bone_threshold *= slope;
	if (otf_auto_mode)
	{
		tissue_threshold *= slope;
	}
	else
	{
		tissue_threshold = kBoneHU - intercept;
	}
#endif
#endif

	air_threshold += otf_additional_offset;
	tissue_threshold += otf_additional_offset;
	bone_threshold += otf_additional_offset;

	vol->setThreshold(air_threshold, tissue_threshold, bone_threshold);

	common::Logger::instance()->Print(common::LogType::INF, QString("threshold a/t/b : %1 / %2 / %3").arg(air_threshold)
		.arg(tissue_threshold)
		.arg(bone_threshold)
		.toStdString());

	bool auto_set_teeth_roi = settings.value("PANORAMA/auto_set_teeth_roi", true).toBool();

	if (auto_set_teeth_roi)
	{
		CW3DetectSliceLoc::run(vol, vol->getAirTissueThreshold(), vol->getTissueBoneThreshold(), &slice_loc);
	}
	else
	{
		// (임시) slice location 이 자동으로 안잡히는 경우가 많음
		// 턱에서 최대 100mm 으로 고정
		float total_length = vol->depth() * vol->sliceSpacing();
		float roi_max_length = 100;

		if (total_length > roi_max_length)
		{
			slice_loc.chin = CW3DetectSliceLoc::FindChinSliceLocation(vol);
			slice_loc.nose = slice_loc.chin - static_cast<int>(roi_max_length / vol->sliceSpacing());
		}
		else
		{
			slice_loc.chin = vol->depth() - 1;
			slice_loc.nose = 0;
		}

		slice_loc.teeth = (slice_loc.chin + slice_loc.nose) / 2;
		//slice_loc.maxilla = (slice_loc.teeth + slice_loc.nose) / 2;
		slice_loc.maxilla = slice_loc.teeth;
		slice_loc.segment_maxilla_mandible = true;
	}

	if (slice_loc.nose == slice_loc.chin)
	{
		slice_loc.chin = vol->depth() - 1;
		slice_loc.nose = 0;

		slice_loc.teeth = (slice_loc.chin + slice_loc.nose) / 2;
		slice_loc.maxilla = slice_loc.teeth;
		slice_loc.segment_maxilla_mandible = true;
	}

	vol->setSliceLoc(slice_loc);

	common::Logger::instance()->Print(
		common::LogType::INF,
		QString(
			"slice location m/t/c/n : %1 / %2 / %3 / %4"
		).arg(slice_loc.maxilla)
		.arg(slice_loc.teeth)
		.arg(slice_loc.chin)
		.arg(slice_loc.nose)
		.toStdString()
	);

	bool use_fixed_value = settings.value("WINDOWING/use_fixed_value", false).toBool();
	if (use_fixed_value)
	{
		int fixed_window_level = settings.value("WINDOWING/fixed_level", 1000).toInt();
		int fixed_window_width = settings.value("WINDOWING/fixed_width", 4000).toInt();
		vol->setWindowing(fixed_window_level - vol->intercept(), fixed_window_width);
	}

	common::Logger::instance()->Print(common::LogType::INF, "end PostProcessing : " + QString::number(timer.elapsed()).toStdString() + " ms");
}

bool CW3DicomIOImpl::ReadPixelData(CW3Image3D*& volume, const int x_start, const int y_start)
{
	if (m_strFileNames.size() < 2)
	{
		return false;
	}

	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::PERCENT);
	progress->setRange(0, 100);
	progress->setValue(0);

	QElapsedTimer timer;
	timer.start();
	common::Logger::instance()->Print(common::LogType::INF, "start ReadPixelData");

	ushort histogram_min = (std::numeric_limits<ushort>::max)();
	ushort histogram_max = (std::numeric_limits<ushort>::min)();
	ushort pixel_min = histogram_min;
	ushort pixel_max = histogram_max;

	CW3ImageHeader* dicom_header = volume->getHeader();

	DcmFileFormat file_format;
	OFCondition condition;

	condition = file_format.loadFile(m_strFileNames.at(0).c_str());
	if (condition.bad())
	{
		qDebug() << condition.text();
		return false;
	}
	DcmDataset* dataset = file_format.getDataset();
	OFString value;
	dataset->findAndGetOFString(DCM_Rows, value);
	dataset->findAndGetOFString(DCM_Columns, value);

	int rows = atoi(value.c_str());
	int columns = atoi(value.c_str());

	int read_area_x = x_start;
	int read_area_y = y_start;
	int read_area_width = volume->width();
	int read_area_height = volume->height();

	int bits_stored = volume->bits_stored();

	QElapsedTimer sub_timer;
	sub_timer.start();

	ushort min = std::numeric_limits<ushort>::max();
	ushort max = std::numeric_limits<ushort>::min();

	std::vector<ushort> min_in_thread;
	std::vector<ushort> max_in_thread;

	bool result = true;

	int old_progress = 0;
	int new_progress = 0;
	int process = 0;

	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

	min_in_thread.resize(num_thread, min);
	max_in_thread.resize(num_thread, max);

#pragma omp parallel for
	for (int i = 0; i < m_strFileNames.size(); ++i)
	{
		int thread_num = omp_get_thread_num();

		ushort* pixel_data = nullptr;
		DicomImage dicom_image(m_strFileNames.at(i).c_str());
		if (dicom_image.getStatus() != EIS_Normal)
		{
			result = false;
			continue;
		}

		pixel_data = (Uint16*)(dicom_image.getOutputData(bits_stored));

		if (!pixel_data)
		{
			result = false;
			continue;
		}

		ushort* slice_data = volume->getData()[i];
		for (int j = 0; j < read_area_height; ++j)
		{
			int y_index = (j + read_area_y) * rows;
			for (int k = 0; k < read_area_width; ++k)
			{
				ushort pixel = pixel_data[y_index + (k + read_area_x)];

				if (pixel < histogram_min)
				{
					histogram_min = pixel;
				}
				else if (pixel > histogram_max)
				{
					histogram_max = pixel;
				}

				pixel *= volume->slope();

				slice_data[j * read_area_width + k] = pixel;

				if (min_in_thread[thread_num] > pixel)
				{
					min_in_thread[thread_num] = pixel;
				}
				if (max_in_thread[thread_num] < pixel)
				{
					max_in_thread[thread_num] = pixel;
				}
			}
		}

		++process;

		new_progress = (int)(((float)process / (float)(m_strFileNames.size() - 1)) * 100.0f);
		if (old_progress != new_progress)
		{
			progress->setValue(new_progress);
			old_progress = new_progress;
		}
	}

	for (int i = 0; i < num_thread; ++i)
	{
		if (min > min_in_thread[i])
		{
			min = min_in_thread[i];
		}
		if (max < max_in_thread[i])
		{
			max = max_in_thread[i];
		}
	}

	qDebug() << "min / max :" << min << "/" << max;
	qDebug() << "get data elapsed_time :" << sub_timer.elapsed();

	if (!result)
	{
		return false;
	}

	ushort** data = volume->getData();

	progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

#if 1
	const auto& mapIter = dicom_header->getListCore().find("PixelRepresentation");
	QString pixel_representation = mapIter->second;

	if (pixel_representation.toInt())
	{
		int signed_short_max = pow(2, bits_stored) / 2 - 1;
		int unsigned_air_hu = signed_short_max - 2000;
		sub_timer.restart();

#if 1
		if (min < unsigned_air_hu)
		{
			min = unsigned_air_hu;
		}

		volume->set_pixel_representation_offset(signed_short_max - min);

#pragma omp parallel for
		for (int z = 0; z < volume->depth(); ++z)
		{
			for (int xy = 0; xy < volume->width() * volume->height(); ++xy)
			{
				data[z][xy] = (data[z][xy] < min) ? 0 : data[z][xy] - min;
			}
		}
#else
		for (int z = 0; z < volume->depth(); ++z)
		{
			for (int xy = 0; xy < volume->width() * volume->height(); ++xy)
			{
				data[z][xy] -= min;
			}
		}
#endif

		ushort sacled_min = min - min;
		ushort sacled_max = max - min;

		volume->setMinMax(sacled_min, sacled_max);

		qDebug() << "rescale elapsed_time :" << sub_timer.elapsed();

		qDebug() << "original signed min / max :" << min - signed_short_max << max - signed_short_max;
		qDebug() << "original unsigned min / max :" << min << max;
		qDebug() << "scaled min / max :" << sacled_min << sacled_max;

		// signed short : -32768 ~ 32767
		// unsigned short : 0 ~ 65535
		// dcmtk 에서 signed -> unsigned 로 linear rescale 되었기 때문에 min - 32768 이 원래 값이다.
		// linear rescale : unsigned value = signed value + 32768
		float original_signed_min = static_cast<float>(min - signed_short_max);
		// rescale intercept 는 unsigned 로 변경된 값을 실제 HU 로 다시 변환하기 위한 값이므로 더해준다.
		volume->setIntercept(volume->intercept() + original_signed_min);
		volume->setWindowing(volume->windowCenter() - volume->intercept(), volume->windowWidth());
	}
	else
	{
		volume->setMinMax(min, max);
	}
#else
	volume->setMinMax(min, max);
#endif

	qDebug() << "window center :" << volume->windowCenter();
	qDebug() << "window width :" << volume->windowWidth();
	qDebug() << "rescale intercept :" << volume->intercept();
	qDebug() << "rescale slope :" << volume->slope();

	sub_timer.restart();

	int histogram_size = max - min + 1;
	int* histogram = SAFE_ALLOC_1D(int, histogram_size);
	std::memset(histogram, 0, sizeof(int) * histogram_size);
	qDebug() << "histogram size :" << histogram_size;

	int depth = volume->depth();
	int xy_size = volume->width() * volume->height();

#pragma omp parallel for
	for (int z = 0; z < depth; ++z)
	{
		for (int xy = 0; xy < xy_size; ++xy)
		{
			ushort value = data[z][xy];
#pragma omp flush(histogram)
			++histogram[static_cast<int>(value)];
		}
	}

#if 1
	int background_index = 0;
#pragma omp parallel for
	for (int i = 0; i < histogram_size; ++i)
	{
#pragma omp flush(background_index)
		if (histogram[background_index] < histogram[i])
		{
			background_index = i;
		}
		//#pragma omp flush(background_index)
	}

	histogram[background_index] = 0;
	histogram[0] = 0;
#endif

#if 0
	std::vector<int> smooth_histogram;
	Common::SmoothingHisto(histogram, histogram_size, smooth_histogram, 100, 7, false);
	volume->setHistogram(smooth_histogram.data(), histogram_size);
#else
	volume->setHistogram(histogram, histogram_size);
#endif
	SAFE_DELETE_ARRAY(histogram);
	qDebug() << "histogram elapsed_time :" << sub_timer.elapsed();

	DJDecoderRegistration::cleanup();

	AutoWindowing(volume);

	common::Logger::instance()->Print(common::LogType::INF, "end ReadPixelData : " + QString::number(timer.elapsed()).toStdString() + " ms");

	return true;
}

bool CW3DicomIOImpl::GetPixelData(const std::string& file, ushort* output, const int bits_stored)
{
	DcmFileFormat file_format;
	OFCondition condition;

	condition = file_format.loadFile(file.c_str());
	if (condition.bad())
	{
		qDebug() << condition.text();
		return false;
	}

	DcmDataset* dataset = file_format.getDataset();
	Uint16* data = nullptr;

	DicomImage* dicom_image = new DicomImage(file.c_str());
	if (!dicom_image)
	{
		return false;
	}

	if (dicom_image->getStatus() != EIS_Normal)
	{
		SAFE_DELETE_OBJECT(dicom_image);
		return false;
	}

	data = (Uint16*)(dicom_image->getOutputData(bits_stored));

	if (!data)
	{
		return false;
	}

	OFString value;
	int rows = 0;
	int columns = 0;
	dataset->findAndGetOFString(DCM_Rows, value);
	rows = atoi(value.c_str());
	dataset->findAndGetOFString(DCM_Columns, value);
	columns = atoi(value.c_str());

	int copy_size = rows * columns * sizeof(Uint16);
	if (copy_size < 1)
	{
		return false;
	}

	memcpy_s(output, copy_size, data, copy_size);

	SAFE_DELETE_OBJECT(dicom_image);

	return true;
}

void CW3DicomIOImpl::AutoWindowing(CW3Image3D* volume)
{
	int* histogram = volume->getHistogram();
	int histogram_size = volume->getHistoSize();
	int window_width = volume->windowWidth();
	if (window_width <= 1)
	{
		int window_center = volume->windowCenter();
		int max_histogram = 0;

#pragma omp parallel for
		for (int i = 1; i < histogram_size - 1; ++i)
		{
#pragma omp flush(window_center, max_histogram)
			if (i < histogram_size * 0.5 &&
				max_histogram < histogram[i])
			{
				max_histogram = histogram[i];
#pragma omp flush(window_center, max_histogram)
				window_center = i;
#pragma omp flush(window_center, max_histogram)
			}
		}
		window_center *= 2;

		window_width = static_cast<int>(static_cast<float>(window_center) * 1.5f);

		volume->setWindowing(window_center - volume->intercept(), window_width);
	}
}

bool CW3DicomIOImpl::PutAndInsertString(DcmDataset* dataset, HeaderList& header, const DcmTag& tag, const std::string& str)
{
	if (header.find(str) != header.end())
	{
		dataset->putAndInsertString(tag, header.at(str).toLocal8Bit().data());
		return true;
	}
	return false;
}

template <typename T>
void CW3DicomIOImpl::Read16BitSlices(CW3Image3D *vol,
	const std::vector<std::string> &file_names,
	const int &x_start, const int &y_start, const int &x_end,
	const int &y_end)
{
	if (typeid(T) != typeid(short) && typeid(T) != typeid(unsigned short))
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			"image is not 16bit data type.");
		throw CW3DicomIOException(CW3DicomIOException::EID::INVALID_VOLUME,
			"image is not 16bit data type.");
	}

	const int bufNBits = 16;
	double val_min = std::numeric_limits<double>::max();
	double val_max = std::numeric_limits<double>::min();

	const float slope = vol->slope();
	const float intercept = vol->intercept();

	qDebug() << "intercept :" << intercept;
	qDebug() << "slope :" << slope;

	int depth = file_names.size();

	QElapsedTimer sub_timer;
	sub_timer.start();

	// Get min / max
	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

#pragma omp parallel for
	for (int i = 0; i < depth; i++)
	{
		DicomImage dicom_image(file_names.at(i).c_str());
		if (dicom_image.getStatus() != EIS_Normal)
		{
			common::Logger::instance()->Print(common::LogType::ERR, "failed to open dicom image.");
			throw CW3DicomIOException(CW3DicomIOException::EID::INVALID_VOLUME, "failed to open image.");
		}

		double tmp_min = std::numeric_limits<double>::max();
		double tmp_max = std::numeric_limits<double>::min();
		dicom_image.getMinMaxValues(tmp_min, tmp_max);

		val_min = std::min(tmp_min, val_min);
		val_max = std::max(tmp_max, val_max);

		//qDebug() << "min / max :" << tmp_min << "/" << tmp_max;
	}

	qDebug() << "min / max :" << val_min << "/" << val_max;
	qDebug() << "get min / max elapsed_time :" << sub_timer.elapsed();

	int histogram_size = val_max - val_min + 1;
	int* histogram = SAFE_ALLOC_1D(int, histogram_size);
	std::memset(histogram, 0, sizeof(int) * histogram_size);

	qDebug() << "histogram size :" << histogram_size;

	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::PERCENT);
	progress->setRange(0, 100);
	progress->setValue(0);

	sub_timer.restart();

	int progress_prev = 0;
	int progress_curr = 0;

	int process = 0;
#pragma omp parallel for
	for (int i = 0; i < depth; i++)
	{
		//#pragma omp atomic
		++process;

		progress_curr = (int)(((float)process / (float)depth) * 100.0f);
		if (progress_prev != progress_curr)
		{
			progress->setValue(progress_curr);
			progress_prev = progress_curr;
		}

		DicomImage dicom_image(file_names.at(i).c_str());
		if (dicom_image.getStatus() != EIS_Normal)
		{
			common::Logger::instance()->Print(common::LogType::ERR,
				"failed to open dicom image.");
			throw CW3DicomIOException(CW3DicomIOException::EID::INVALID_VOLUME,
				"failed to open image.");
		}

		const void *frame_data;

		int width = dicom_image.getWidth();
		int height = dicom_image.getHeight();

		unsigned short *data_slice = vol->getData()[i];

		if (typeid(T) == typeid(short))
		{
			const DiMonoPixel *pixels =
				(const DiMonoPixel *)dicom_image.getInterData();
			if (pixels == NULL)
			{
				common::Logger::instance()->Print(common::LogType::ERR,
					"failed to open DiMonoPixel.");
				throw CW3DicomIOException(CW3DicomIOException::EID::INVALID_VOLUME,
					"failed to open image.");
			}

			frame_data = pixels->getData();
			if (frame_data == NULL)
			{
				throw CW3DicomIOException(CW3DicomIOException::EID::READ_ERROR,
					"could not get data.");
			}
			for (int j = 0; j < y_end; j++)
			{
				const T *data = static_cast<const T *>(frame_data) +
					(y_start + j) * width + x_start;
				for (int k = 0; k < x_end; k++)
				{
					int cast_val =
						static_cast<int>(static_cast<float>(*data++) - intercept);

					if (cast_val < 0) cast_val = 0;

					if (cast_val >= histogram_size)
					{
						cast_val = histogram_size - 1;
					}

					unsigned short val =
						static_cast<unsigned short>(slope * static_cast<float>(cast_val));
					*data_slice++ = val;
#if APPLY_SLOPE_TO_HISTOGRAM
					++histogram[static_cast<int>(slope * static_cast<float>(cast_val))];
#else
					++histogram[cast_val];
#endif
				}
			}
		}
		else
		{
			frame_data = dicom_image.getOutputData(vol->bits_stored(), 0);

			if (frame_data == NULL)
			{
				throw CW3DicomIOException(CW3DicomIOException::EID::READ_ERROR,
					"could not get data.");
			}

			for (int j = 0; j < y_end; j++)
			{
				const T *data = static_cast<const T *>(frame_data) + (y_start + j) * width + x_start;

				for (int k = 0; k < x_end; k++)
				{
					int cast_val = static_cast<int>(*data++);

					if (cast_val < 0) cast_val = 0;

					if (cast_val >= histogram_size)
					{
						cast_val = histogram_size - 1;
					}

					unsigned short val =
						static_cast<unsigned short>(slope * static_cast<float>(cast_val));
					*data_slice++ = val;

#if APPLY_SLOPE_TO_HISTOGRAM
					++histogram[static_cast<int>(slope * static_cast<float>(cast_val))];
#else
					++histogram[cast_val];
#endif
				}
			}
		}
	}

	qDebug() << "get data elapsed_time :" << sub_timer.elapsed();

	unsigned short min;
	unsigned short max;

#if 1
	if (typeid(T) == typeid(short))
	{
		min = (unsigned short)(0);
		max = (unsigned short)((float)val_max - intercept);
	}
	else
	{
		min = (unsigned short)((float)val_min - intercept);
		max = (unsigned short)((float)val_max - intercept);
	}
#else
	min = static_cast<unsigned short>(static_cast<float>(val_min) + intercept);
	max = static_cast<unsigned short>(static_cast<float>(val_max) + intercept);
#endif

	vol->setMinMax(min, max);
	//vol->setHistogram(histogram, (int)((float)(max - min) / vol->slope()) + 1);

	// histogram smoothing
	std::vector<int> smooth_histo;
#if 0
	Common::SmoothingHisto(histogram, histogram_size, smooth_histo, 100, 7, false);
#else
	smooth_histo.resize(histogram_size, 0);
	for (int i = 0; i < histogram_size; ++i)
	{
		smooth_histo[i] = histogram[i];
	}
#endif
	vol->setHistogram(smooth_histo.data(), smooth_histo.size());

	qDebug() << "min / max :" << min << "/" << max;

	// auto windowing
	int window_width = vol->windowWidth();
	if (window_width <= 1)
	{
		int window_center = vol->windowCenter();
		int max_histogram = 0;
		int background = 0;

#pragma omp parallel for
		for (int i = 1; i < histogram_size - 1; ++i)
		{
			if (i < histogram_size * 0.5 &&
				max_histogram < histogram[i])
			{
				max_histogram = histogram[i];
				window_center = i;
			}

			if (histogram[background] < histogram[i])
			{
				background = i;
			}
		}
		window_center *= 2;

		window_width = static_cast<int>(static_cast<float>(window_center) * 1.5f);
		window_center += background;

		vol->setWindowing(window_center - vol->intercept(), window_width);
	}

	SAFE_DELETE_ARRAY(histogram);
}
