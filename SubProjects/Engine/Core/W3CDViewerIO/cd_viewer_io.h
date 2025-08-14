#pragma once 
/*=========================================================================

File:          cd_viewer_io.h
Language:      C++11
Library:       Qt 5.8, Standard C++ Library
Author:        Seo Seok Man
First Date:    2017-07-21
Last Modify:   2017-08-24

Copyright (c) 2017 All rights reserved by HDXWILL.

=========================================================================*/
#include <string>
#include <vector>
#include <map>

#include "w3cdviewerio_global.h"

class CW3Image3D;

/**************************************************************************************************
 * @class	W3CDViewerIO
 *
 * @brief	CD Viewer 에 쓰일 Raw Data를 만드는 클래스 
 * 			현재는 DICOM data 만 만들고, conf.dat 라는 이름으로 저장된다.
 * 			
 *	[사용 방법] 
 *		Import 시 : W3CDViewerIO::ImportRawDCM(volume);
 *		Export 시 : W3CDViewerIO::ExportRawDCM(volume);
 *		CW3FILEtab::importDICOM, CW3FILEtab::exportDICOM 참조
 *		TODO(smseo) : 압축 모드에 대해 작성
 *
 * @author	Seo Seok Man
 * @date	2017-08-24
 **************************************************************************************************/

class W3CDVIEWERIO_EXPORT W3CDViewerIO 
{
public:
	W3CDViewerIO() {} 
	~W3CDViewerIO() {}

	W3CDViewerIO(const W3CDViewerIO&) = delete;
	W3CDViewerIO& operator=(const W3CDViewerIO&) = delete;

public:
	static const bool ExportRawDCM(const CW3Image3D& volume, const QString& selected_path,
								   const bool& is_compressed, const std::vector<std::string>& dcm_path);
	static const bool ExportDCMOnly(const CW3Image3D& volume, const QString& selected_path,
									const std::vector<std::string>& dcm_path);
	static const bool ImportRawDCM(CW3Image3D*& volume);

	static QByteArray GetCyperText(const CW3Image3D& volume, const bool compressed);
	
private:
	static std::vector<std::string> GetHeader(std::map<std::string,
											  std::string>& listHeader,
											  CW3Image3D*& vol);
	static bool ExportCypherText(const QByteArray& cypher_text,
								 const QString& selected_path,
								 const QString& export_path, const bool& is_compressed);
	static QByteArray ImportCypherText(bool& is_compressed, bool& result);
	static QByteArray* ImportCypherTextPointer(bool& is_compressed, bool& result);
	
};
