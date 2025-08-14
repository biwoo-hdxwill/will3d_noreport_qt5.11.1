#include "cd_viewer_io.h"

#ifndef _WIN64
#include <Windows.h>
#include <psapi.h>
#endif

#include <QDebug>
#include <qbuffer.h>
#include <qfile.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qprocess.h>
#include <QCoreApplication>
#include <windows.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <objbase.h>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/large_address_aware_win32.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ImageHeader.h"
#include "../../Resource/ResContainer/resource_container.h"

#include "QSimpleCrypt.h"

namespace
{
	const int kDefaultMaxHeapSize = (500 * 1024 * 1024); // 500MB

	const QString kMasterKey("HDXWILLSoftwareKey");
	const QString kRawFileImportPath("./conf.dat"); // non-compressed
	const QString kRawFileImportPathComp("./cconf.dat"); // compressed
	const QString kRawFileName("/conf.dat"); // non-compressed
	const QString kRawFileNameComp("/cconf.dat"); // compressed

	qint64 GetKey(const QString& str)
	{
		QByteArray hash = QCryptographicHash::hash(
			QByteArray::fromRawData((const char*)str.utf16(), str.length() * 2),
			QCryptographicHash::Md5
		);
		Q_ASSERT(hash.size() == 16);
		QDataStream stream(hash);
		qint64 a, b;
		stream >> a >> b;
		return a ^ b;
	}

	QString GetLocalQStr(const std::string& msg) { return QString::fromLocal8Bit(msg.c_str()); }

	void WriteHeaderInfo(QDataStream& ds, CW3ImageHeader* pH)
	{
		common::Logger::instance()->Print(common::LogType::INF, "start WriteHeaderInfo");

		ds << pH->getStudyID();
		ds << pH->getSeriesID();
		ds << pH->getPatientID();
		ds << pH->getPatientName();
		ds << pH->getPatientBirthDate();
		ds << pH->getPatientAge();
		ds << pH->getPatientSex();
		ds << pH->getKVP();
		ds << pH->getModality();
		ds << pH->getXRayTubeCurrent();
		ds << pH->getStudyDate();
		ds << pH->getStudyTime();
		ds << pH->getSeriesDate();
		ds << pH->getSeriesTime();
		ds << pH->getDescription();

		common::Logger::instance()->Print(common::LogType::INF, "end WriteHeaderInfo");
	}

	std::map<std::string, QString> ReadHeaderInfo(QDataStream& ds)
	{
		std::map<std::string, QString> header;

		QString str;
		ds >> str;
		header["StudyInstanceUID"] = str;
		ds >> str;
		header["SeriesInstanceUID"] = str;
		ds >> str;
		header["PatientID"] = str;
		ds >> str;
		header["PatientName"] = str;
		ds >> str;
		header["PatientBirthDate"] = str;
		ds >> str;
		header["PatientAge"] = str;
		ds >> str;
		header["PatientSex"] = str;
		ds >> str;
		header["KVP"] = str;
		ds >> str;
		header["Modality"] = str;
		ds >> str;
		header["XRayTubeCurrent"] = str;
		ds >> str;
		header["StudyDate"] = str;
		ds >> str;
		header["StudyTime"] = str;
		ds >> str;
		header["SeriesDate"] = str;
		ds >> str;
		header["SeriesTime"] = str;
		ds >> str;
		header["StudyDescription"] = str;

		return header;
	}

	void WriteVolumeInfo(QDataStream& ds, const CW3Image3D& image, const int down_scale_factor)
	{
		int slice_down_scale_factor = down_scale_factor;
		int depth_down_scale_factor = down_scale_factor;

		common::Logger::instance()->Print(common::LogType::INF, "start WriteVolumeInfo");

		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::PERCENT);
		progress->setRange(0, 100);
		progress->setValue(0);
		int pre_percent = 0, cur_percent = 0;

		// volume data
		unsigned int w = static_cast<unsigned int>(static_cast<float>(image.width()) / slice_down_scale_factor + .5f);
		unsigned int h = static_cast<unsigned int>(static_cast<float>(image.height()) / slice_down_scale_factor + .5f);
		unsigned int d = static_cast<unsigned int>(static_cast<float>(image.depth()) / depth_down_scale_factor + .5f);

		QString volume_size = QString("%1 x %2 x %3").arg(w).arg(h).arg(d);
		common::Logger::instance()->Print(common::LogType::INF, "WriteVolumeInfo volume size : " + volume_size.toStdString());

		ds << w << h << d;
		unsigned short** volume_data = image.getData();
		unsigned short valAt;

		int ori_width = image.width();
		for (int z = 0; z < d; ++z)
		{
			cur_percent = (int)(((float)z / (float)d) * 100.0f);
			if (pre_percent != cur_percent)
			{
				progress->setValue(cur_percent);
				pre_percent = cur_percent;
			}

			int iz = z * depth_down_scale_factor;
			unsigned short* data_line = volume_data[iz];
			for (int y = 0; y < h; ++y)
			{
				int iy = ori_width * y * slice_down_scale_factor;
				for (int x = 0; x < w; ++x)
				{
					int ix = x * slice_down_scale_factor;
					valAt = data_line[iy + ix];
					ds << valAt;
				}
			}
		}

#if 1
		ds << (image.pixelSpacing() * slice_down_scale_factor)
			<< (image.sliceSpacing() * depth_down_scale_factor);
#else
		ds << image.pixelSpacing()
			<< image.sliceSpacing();
#endif
		ds << image.windowCenter()
			<< image.windowWidth();

		ds << image.slope()
			<< image.intercept();

		ds << image.getMin()
			<< image.getMax();

		ds << image.getAirTissueThreshold()
			<< image.getTissueBoneThreshold()
			<< image.getBoneTeethThreshold();

		// slice location
#if 1
		const SliceLoc& SL = image.getSliceLoc();
		int scaled_maxilla = static_cast<int>(static_cast<float>(SL.maxilla) / static_cast<float>(depth_down_scale_factor));
		int scaled_teeth = static_cast<int>(static_cast<float>(SL.teeth) / static_cast<float>(depth_down_scale_factor));
		int scaled_chin = static_cast<int>(static_cast<float>(SL.chin) / static_cast<float>(depth_down_scale_factor));
		int scaled_nose = static_cast<int>(static_cast<float>(SL.nose) / static_cast<float>(depth_down_scale_factor));

		ds << scaled_maxilla
			<< scaled_teeth
			<< scaled_chin
			<< scaled_nose;
#else
		const SliceLoc& SL = image.getSliceLoc();
		ds << SL.maxilla
			<< SL.teeth
			<< SL.chin
			<< SL.nose;
#endif

		// histogram
		int histo_size = image.getHistoSize();
		ds << histo_size;
		int* histogram = image.getHistogram();
		for (int idx = 0; idx < histo_size; ++idx)
			ds << histogram[idx];
	}

	void ReadVolumeInfo(QDataStream& ds, CW3Image3D*& image)
	{
		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::PERCENT);
		progress->setRange(0, 100);
		progress->setValue(0);
		int pre_percent = 0, cur_percent = 0;

		int w, h, d;
		ds >> w >> h >> d;

		image = new CW3Image3D(w, h, d);
		unsigned short** volume_data = image->getData();
		unsigned short valAt;
		for (int z = 0; z < d; ++z)
		{
			cur_percent = (int)(((float)z / (float)d) * 100.0f);
			if (pre_percent != cur_percent)
			{
				progress->setValue(cur_percent);
				pre_percent = cur_percent;
			}

			unsigned short* data_line = volume_data[z];
			for (int xy = 0; xy < w*h; ++xy)
			{
				ds >> valAt;
				*data_line++ = valAt;
			}
		}

		float float_value;
		ds >> float_value; image->setPixelSpacing(float_value);
		ds >> float_value; image->setSliceSpacing(float_value);

		int center, width;
		ds >> center >> width; image->setWindowing(center, width);

		ds >> float_value; image->setSlope(float_value);
		ds >> float_value; image->setIntercept(float_value);

		unsigned short min, max;
		ds >> min >> max; image->setMinMax(min, max);

		int thd_air, thd_tissue, thd_bone;
		ds >> thd_air >> thd_tissue >> thd_bone; image->setThreshold(thd_air, thd_tissue, thd_bone);

		SliceLoc SL;
		ds >> SL.maxilla >> SL.teeth >> SL.chin >> SL.nose; image->setSliceLoc(SL);

		int int_value;
		ds >> int_value;
		int* histogram = new int[int_value];
		for (int idx = 0; idx < int_value; ++idx)
			ds >> histogram[idx];
		image->setHistogram(histogram, int_value);
	}

	bool CopyDirRecursive(const QString& from_dir, const QString& to_dir)
	{
		QDir dir;
		dir.setPath(from_dir);
#if 0
		from_dir += QDir::separator();
		to_dir += QDir::separator();
#else
		QString from_ = from_dir + "/";
		QString to_ = to_dir + "/";
#endif

		foreach(QString copy_file, dir.entryList(QDir::Files))
		{
			QString from = from_ + copy_file;
			QString to = to_ + copy_file;

			if (QFile::exists(to))
				if (QFile::remove(to) == false)
					return false;

			if (QFile::copy(from, to) == false)
				return false;
		}

		foreach(QString copy_dir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
		{
			QString from = from_ + copy_dir;
			QString to = to_ + copy_dir;

			if (dir.mkpath(to) == false)
				return false;

			if (CopyDirRecursive(from, to) == false)
				return false;
		}
		return true;
	}
};

const bool W3CDViewerIO::ExportRawDCM(const CW3Image3D& volume, const QString& selected_path, const bool& is_compressed, const std::vector<std::string>& dcm_path)
{
	common::Logger::instance()->Print(common::LogType::INF, "start W3CDViewerIO::ExportRawDCM");

	QByteArray cypher_text = GetCyperText(volume, is_compressed);

	QString input_path = selected_path;

	CW3ImageHeader* H = volume.getHeader();
	QString folder_name = H->getSeriesDate() + "_" + H->getPatientName();
	QString export_path;
	if (input_path.at(input_path.size() - 1) == "/")
	{
		export_path = input_path + folder_name + "/AppFiles";
	}
	else
	{
		export_path = input_path + "/" + folder_name + "/AppFiles";
	}


	common::Logger::instance()->Print(common::LogType::INF, "W3CDViewerIO::ExportRawDCM folder_name : " + folder_name.toStdString());
	common::Logger::instance()->Print(common::LogType::INF, "W3CDViewerIO::ExportRawDCM export_path : " + export_path.toStdString());

	if (!ExportCypherText(cypher_text, input_path, export_path, is_compressed))
	{
		return false;
	}
 
	QString viewer_path = QCoreApplication::applicationDirPath() + "/cd_viewer_bin";
	QDir dir(viewer_path);
	if (dir.exists())
	{
		if (!CopyDirRecursive(viewer_path, export_path))
		{
			return false;
		}
	}

	common::Logger::instance()->Print(common::LogType::INF, "CW3DicomIOImpl::exportRawDCM : cd viewer copy success");

  // 20250613 wbi .dcm 원본 export
  if (!dcm_path.empty())
  {
    QString dicom_folder_path = input_path;
    if (input_path.at(input_path.size() - 1) != "/")
    {
      dicom_folder_path += "/";
    }
    dicom_folder_path += folder_name + "/DICOM";

    QDir dicom_dir(dicom_folder_path);
    if (!dicom_dir.exists())
    {
      if (!QDir().mkpath(dicom_folder_path))
      {
        common::Logger::instance()->Print(common::LogType::WRN, "Failed to create DICOM directory");
      }
      else
      {
        common::Logger::instance()->Print(common::LogType::INF, "Copying original DICOM files...");

        bool copy_success = true;
        for (const auto& dcm_at : dcm_path)
        {
          QString dcm_qstr = QString::fromLocal8Bit(dcm_at.c_str());
          QFileInfo file_info(dcm_qstr);
          QString dest_path = dicom_folder_path + "/" + file_info.fileName();

          if (!QFile::copy(dcm_qstr, dest_path))
          {
            common::Logger::instance()->Print(common::LogType::WRN,
              "Failed to copy DICOM file: " + file_info.fileName().toStdString());
            copy_success = false;
          }
        }

        if (copy_success)
        {
          common::Logger::instance()->Print(common::LogType::INF,
            QString("Successfully copied %1 DICOM files").arg(dcm_path.size()).toStdString());
        }
      }
    }
  }
	common::Logger::instance()->Print(common::LogType::INF, "end W3CDViewerIO::ExportRawDCM");

	QString file_path = export_path + "/Will3D.exe";
	QFile file(file_path);
	if (file.exists())
	{
		QString link_path = selected_path + "/" + folder_name + "/Will3D_Viewer.lnk";

    file.setFileName(export_path + "/Will3D_Viewer.lnk");
    if(file.exists())
      file.copy(link_path);
	}

	return true;
}


QByteArray W3CDViewerIO::GetCyperText(const CW3Image3D& volume, const bool compressed)
{
	int width = volume.width();
	int height = volume.height();
	int depth = volume.depth();
	int volume_size = width * height * depth * sizeof(unsigned short);
	int down_scale_factor = 1;

#if 1
#if 0
	if (kDefaultMaxHeapSize < volume_size)
	{
		float ratio = static_cast<float>(volume_size) / kDefaultMaxHeapSize;
		down_scale_factor = ceil(ratio);
	}
#else

	MEMORYSTATUSEX memory;
	ZeroMemory(&memory, sizeof(MEMORYSTATUSEX));
	memory.dwLength = sizeof(MEMORYSTATUSEX);

	GlobalMemoryStatusEx(&memory);
	DWORDLONG ull_avail_phys = memory.ullAvailPhys/* >> 10*/;

#ifdef _WIN64
	if (ull_avail_phys < volume_size)
	{
		float ratio = static_cast<float>(volume_size) / ull_avail_phys;
		down_scale_factor = ceil(ratio);
	}
#else
	//typedef struct _PROCESS_MEMORY_COUNTERS {
	//	DWORD cb							//구조체 크기
	//	DWORD PageFaultCount;				//페이지 폴트 개수
	//	SIZE_T PeakWorkingSetSize;			//최대 워킹셋 크기(byte)
	//	SIZE_T WorkingSetSize;				//현재 워킹셋 크기(byte)
	//	SIZE_T QuotaPeakPagedPoolUsage;		//최대 페이지 풀 사용량(byte)
	//	SIZE_T QuotaPagedPoolUsage;			//현재 페이지 풀 사용량(byte)
	//	SIZE_T QuotaPeakNonPagedPoolUsage;	//최대 넌페이지 풀 사용량(byte)
	//	SIZE_T QuotaNonPagedPoolUsage;		//현재 넌페이즈 풀 사용량(byte)
	//	SIZE_T PagefileUsage;				//할당된 메모리량(byte)
	//	SIZE_T PeakPagefileUsage;			//최대 할당된 메모리량(byte)
	//} PROCESS_MEMORY_COUNTERS;

	int size_option = 2;
	if (LargeAddressAwareWin32::GetInstance()->large_address_aware())
	{
		size_option = 3;
	}

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	int working_size = pmc.WorkingSetSize;

	int max_memory_size = 1024 * 1024 * 1024 * size_option;
	int max_volume_size = 1024 * 1024 * 1024;

	int volume_down_factor = 1.f;
	if (max_volume_size < volume_size)
	{
		float ratio = static_cast<float>(volume_size) / max_volume_size;
		volume_down_factor = ceil(ratio);
	}

	int process_available_maximum_size = max_memory_size - working_size;
	if (process_available_maximum_size < ull_avail_phys)
	{
		if (process_available_maximum_size < volume_size / volume_down_factor)
		{
			float ratio = static_cast<float>(volume_size / volume_down_factor) / process_available_maximum_size;
			down_scale_factor = ceil(ratio);
		}
	}
	else
	{
		if (ull_avail_phys < volume_size / volume_down_factor)
		{
			float ratio = static_cast<float>(volume_size / volume_down_factor) / ull_avail_phys;
			down_scale_factor = ceil(ratio);
		}
	}

	down_scale_factor *= volume_down_factor;

#endif // !_WIN64
#endif

#else
	if (kDefaultMaxHeapSize <= volume_size)
	{
		down_scale_factor = 2;
	}
#endif

	QString max_heap_size_log = QString("maximum heap size : %1").arg(kDefaultMaxHeapSize);
	QString volume_size_log = QString("volume size : %1 / down factor : %2").arg(volume_size).arg(down_scale_factor);

	common::Logger::instance()->Print(common::LogType::INF, max_heap_size_log.toStdString());
	common::Logger::instance()->Print(common::LogType::INF, volume_size_log.toStdString());

#if 0
	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);
	QDataStream s(&buffer);
	s.setVersion(QDataStream::Qt_5_8);

	//write header info
	CW3ImageHeader* H = volume.getHeader();
	WriteHeaderInfo(s, H);
	WriteVolumeInfo(s, volume, down_scale_factor);

	QByteArray ba = buffer.data();
	buffer.close();

	int sz = ba.size();
#else
	QByteArray dynamic_ba;
	QBuffer buffer(&dynamic_ba);

	buffer.open(QIODevice::WriteOnly);
	QDataStream s(&buffer);
	s.setVersion(QDataStream::Qt_5_8);

	//write header info
	CW3ImageHeader* H = volume.getHeader();
	WriteHeaderInfo(s, H);
	WriteVolumeInfo(s, volume, down_scale_factor);

	QByteArray ba = buffer.data();
	buffer.close();

	int const_ba = ba.size();
#endif
	auto key = GetKey(kMasterKey);

	// encryption dicom data
	SimpleCrypt cryptor(key);
	if (compressed)
	{
		cryptor.setCompressionMode(SimpleCrypt::CompressionAlways);
	}
	else
	{
		cryptor.setCompressionMode(SimpleCrypt::CompressionNever);
	}

	cryptor.setIntegrityProtectionMode(SimpleCrypt::ProtectionHash);

	QByteArray cypher_text = cryptor.encryptToByteArray(ba);
	if (cryptor.lastError() != SimpleCrypt::ErrorNoError)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "CW3DicomIOImpl::exportRawDCM : encryption failed");
		return QByteArray();
	}

	return cypher_text;
}

const bool W3CDViewerIO::ExportDCMOnly(const CW3Image3D& volume, const QString& selected_path,
	const std::vector<std::string>& dcm_path)
{
	CW3ImageHeader* H = volume.getHeader();
	QString folder_name = H->getSeriesDate() + "_" + H->getPatientName();
	QString export_path = selected_path + "/" + folder_name;

	QDir export_dir(export_path);
	if (export_dir.exists())
	{
		export_dir.removeRecursively();
	}

	QDir().mkdir(export_path);

	for (const auto& dcm_at : dcm_path)
	{
		QString dcm_qstr = QString::fromLocal8Bit(dcm_at.c_str());
		QFileInfo file_info(dcm_qstr);
		QString full_path_dest = export_path + "/" + file_info.fileName();
		if (QFile::copy(dcm_qstr, full_path_dest) == false)
			return false;
	}
	return true;
}

const bool W3CDViewerIO::ImportRawDCM(CW3Image3D*& volume)
{
	if (volume)
		SAFE_DELETE_OBJECT(volume);

	SimpleCrypt cryptor(GetKey(kMasterKey));

	bool is_compressed, result;
#if 0
	QByteArray cypher_text = ImportCypherText(is_compressed, result);
	if (result == false)
	{
		return false;
	}

	common::Logger::instance()->Print(common::LogType::ERR, "start decryptToByteArray");

	// decryption dicom data
	QByteArray plane_text = cryptor.decryptToByteArray(cypher_text);
	QBuffer buffer_in(&plane_text);
#else
	QByteArray* cypher_text = ImportCypherTextPointer(is_compressed, result);
	if (result == false)
	{
		return false;
	}

	common::Logger::instance()->Print(common::LogType::ERR, "start decryptToByteArray");

	// decryption dicom data
	QByteArray* plane_text = cryptor.decryptToByteArray(cypher_text);
	QBuffer buffer_in(plane_text);
#endif
	if (cryptor.lastError() != SimpleCrypt::ErrorNoError)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "CW3DicomIOImpl::exportRawDCM : decryption failed");
		return false;
	}

	common::Logger::instance()->Print(common::LogType::ERR, "end decryptToByteArray");

	common::Logger::instance()->Print(common::LogType::ERR, "start ReadHeaderInfo");

	// read header info
	//QBuffer buffer_in(&plane_text);
	buffer_in.open(QIODevice::ReadOnly);
	QDataStream read_stream(&buffer_in);
	read_stream.setVersion(QDataStream::Qt_5_8);
	std::map<std::string, QString> HList = ReadHeaderInfo(read_stream);
	ReadVolumeInfo(read_stream, volume);
	buffer_in.close();

	SAFE_DELETE_OBJECT(cypher_text);

	common::Logger::instance()->Print(common::LogType::ERR, "end ReadHeaderInfo");

	std::shared_ptr<CW3ImageHeader> header = std::shared_ptr<CW3ImageHeader>(new CW3ImageHeader(HList));
	volume->setHeader(header);
	ResourceContainer::GetInstance()->SetDicomHeaderResource(header);

	return true;
}

std::vector<std::string> W3CDViewerIO::GetHeader(std::map<std::string, std::string>& hd, CW3Image3D*& vol)
{
	std::vector<std::string> list;
	for (auto &i : hd)
	{
		if (i.first == QString("Rows").toStdString())
			i.second = QString::number(vol->height()).toStdString();

		if (i.first == QString("Columns").toStdString())
			i.second = QString::number(vol->width()).toStdString();

		if (i.first == QString("PatientName").toStdString())
			if (i.first == QString("PatientID").toStdString())
				if (i.first == QString("PatientBirthDate").toStdString())
					if (i.first == QString("PatientSex").toStdString())
					{
						std::string str(i.first + " : " + i.second);
						list.push_back(str);
					}
	}

	return list;
}

bool W3CDViewerIO::ExportCypherText(const QByteArray& cypher_text,
	const QString& selected_path,
	const QString& export_path, const bool& is_compressed)
{
	common::Logger::instance()->Print(common::LogType::INF, "start W3CDViewerIO::ExportCypherText");

	QDir export_dir(export_path);
#if 0
	if (export_dir.exists())
	{
		export_dir.removeRecursively();
	}

	QDir().mkdir(export_path);
	//std::cout << "final cd viewer's directory path : " << export_path->toStdString() << std::endl;
#else
	if (!export_dir.exists())
	{
		if (!QDir().mkpath(export_path))
		{
			return false;
		}
	}
#endif

	QString file_name = is_compressed ? kRawFileNameComp : kRawFileName;
	QString full_path = export_path + file_name;
	QFile file(full_path);
	if (file.exists())
	{
		QFile::remove(full_path);
	}

	if (!file.open(QIODevice::WriteOnly))
	{
		return false;
	}
	file.seek(0);
	QDataStream s(&file);
	s << cypher_text;
	file.close();

	common::Logger::instance()->Print(common::LogType::INF, "end W3CDViewerIO::ExportCypherText");

	return true;
}

QByteArray W3CDViewerIO::ImportCypherText(bool& is_compressed, bool& result)
{
	QByteArray cypher_text;
	QFile file(kRawFileImportPath);
	if (file.open(QIODevice::ReadOnly))
	{
		common::Logger::instance()->Print(common::LogType::ERR, "W3CDViewerIO::ImportCypherText : raw");
#if 1
		cypher_text = file.readAll();
		cypher_text.remove(0, 4);
#else
		file.seek(0);
		QDataStream s(&file);
		s >> cypher_text;
#endif
		is_compressed = false;
	}
	else
	{
		common::Logger::instance()->Print(common::LogType::ERR, "W3CDViewerIO::ImportCypherText : compressed");

		QFile file_comp(kRawFileImportPathComp);
		if (!file_comp.open(QIODevice::ReadOnly))
		{
			common::Logger::instance()->Print(common::LogType::ERR,
				"W3CDViewerIO::ImportCypherText : file does not exist.");
			result = false;
			return cypher_text;
		}
		file_comp.seek(0);
		QDataStream s(&file_comp);
		s >> cypher_text;
		is_compressed = true;
	}

	common::Logger::instance()->Print(common::LogType::ERR, "W3CDViewerIO::ImportCypherText : closed");

	file.close();
	result = true;
	return cypher_text;
}

QByteArray* W3CDViewerIO::ImportCypherTextPointer(bool& is_compressed, bool& result)
{
	QByteArray* cypher_text = new QByteArray();
	QFile file(kRawFileImportPath);
	if (file.open(QIODevice::ReadOnly))
	{
		common::Logger::instance()->Print(common::LogType::ERR, "W3CDViewerIO::ImportCypherText : raw");

		*cypher_text = file.readAll();
		cypher_text->remove(0, 4);

		is_compressed = false;
	}
	else
	{
		common::Logger::instance()->Print(common::LogType::ERR, "W3CDViewerIO::ImportCypherText : compressed");

		QFile file_comp(kRawFileImportPathComp);
		if (!file_comp.open(QIODevice::ReadOnly))
		{
			common::Logger::instance()->Print(common::LogType::ERR,
				"W3CDViewerIO::ImportCypherText : file does not exist.");
			result = false;
			return cypher_text;
		}
		file_comp.seek(0);
		QDataStream s(&file_comp);
		s >> *cypher_text;
		is_compressed = true;
	}

	common::Logger::instance()->Print(common::LogType::ERR, "W3CDViewerIO::ImportCypherText : closed");

	file.close();
	result = true;
	return cypher_text;
}
