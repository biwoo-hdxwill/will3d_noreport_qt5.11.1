#include "W3Implant.h"
/*=========================================================================

File:			class CW3Implant
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-05-20
Last date:		2016-05-20

=========================================================================*/
#include <ctime>
#include <iostream>

#include <QFile>
#include <qmath.h>
#include <QFileInfo>

#include <Engine/Common/Common/language_pack.h>
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/GLfunctions/W3GLTypes.h"

class Vec3Less {
public:
	bool operator()(const glm::vec3& v0, const glm::vec3& v1) const {
		return v0.x < v1.x ||
			(v0.x == v1.x && v0.y < v1.y) ||
			(v0.x == v1.x && v0.y == v1.y && v0.z < v1.z);
	}
};

CW3Implant::CW3Implant(QOpenGLWidget *GLWidget, int id, float diameter, float length, bool useOBB)
	: m_pgGLWidget(GLWidget), m_id(id), m_diameter(diameter), m_length(length)/*, m_bUseOBB(useOBB)*/ {
	init();
}

CW3Implant::~CW3Implant() {
	clearGLresources();
}

void CW3Implant::init() {
	m_vbo[0] = 0;
	m_vbo[1] = 0;
	m_vbo[2] = 0;

	m_RightVector = glm::vec3(1.0f, 0.0f, 0.0f);
	m_BackVector = glm::vec3(0.0f, 1.0f, 0.0f);
	m_UpVector = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::mat4 rotateMatrix(1.0f);

	m_rotate = glm::mat4(1.0f);
	m_translate = glm::mat4(1.0f);
	m_scaleMatrix = glm::mat4(1.0f);
	if (m_id >= 28 / 2) {
		rotateMatrix = glm::rotate(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		m_rotate = glm::rotate(float(M_PI), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	m_model = m_rotate * m_translate*m_scaleMatrix;

	m_scaleMatrix = glm::mat4(1.0f);

	m_GLtoTex = glm::scale(glm::vec3(0.5f))*glm::translate(glm::vec3(1.0f))
		* glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f));
}

void CW3Implant::clearGLresources() {
	if (m_pgGLWidget->context()) {
		m_pgGLWidget->makeCurrent();
		if (m_vbo[0]) {
			glDeleteBuffers(3, m_vbo);
			m_vbo[0] = 0;
			m_vbo[1] = 0;
			m_vbo[2] = 0;
		}

		m_pgGLWidget->doneCurrent();
	}
}

bool CW3Implant::implantLoad(const QString& manufacturerName,
							 const QString& productName,
							 const QString& stlFileName) {
	implant_path_ = stlFileName;
	manufacturer_name_ = manufacturerName;
	product_name_ = productName;

	clock_t startTime = clock();

	vert_list_.clear();
	index_list_.clear();
	normal_list_.clear();

	std::map<glm::vec3, std::vector<int>, Vec3Less> mapVertIdx;
	std::vector<std::pair<Triangle, int>> triForZorder;
	//std::map<glm::vec3, int, Vec3Less> mapVertIdx;
	int indexCount = 0;

	QFile fileSTL(stlFileName);
	printf("implantLoad : %s\r\n", stlFileName.toStdString().c_str());

	if (!fileSTL.open(QIODevice::ReadOnly)) {
		printf("%s : load implant file has failed.\r\n", stlFileName.toStdString().c_str());
		return false;
	}
	fileSTL.seek(0);

	QDataStream datastream(&fileSTL);
	datastream.setByteOrder(QDataStream::LittleEndian);
	datastream.setFloatingPointPrecision(QDataStream::SinglePrecision);
	fileSTL.seek(80);

	quint32 nTriangles;
	datastream >> nTriangles;

	quint32 nCountTriangles = 0;

	bb_min_ = glm::vec3(10000, 10000, 10000);
	bb_max_ = glm::vec3(-10000, -10000, -10000);

	std::vector<glm::vec3>	vFaceNormals;

	quint16 nControlBytes;
	while (nCountTriangles < nTriangles) {
		float nx, ny, nz, x[3], y[3], z[3];
		// nx, ny, nz: normal for face
		fileSTL.seek(84 + nCountTriangles * 50 + 0 + 0);
		datastream >> nx;
		fileSTL.seek(84 + nCountTriangles * 50 + 0 + 4);
		datastream >> ny;
		fileSTL.seek(84 + nCountTriangles * 50 + 0 + 8);
		datastream >> nz;

		// x[0], y[0], z[0]: x, y, z coordinate for each triangle
		fileSTL.seek(84 + nCountTriangles * 50 + 12 + 0);
		datastream >> x[0];
		fileSTL.seek(84 + nCountTriangles * 50 + 12 + 4);
		datastream >> y[0];
		fileSTL.seek(84 + nCountTriangles * 50 + 12 + 8);
		datastream >> z[0];

		// x[1], y[1], z[1]: x, y, z coordinate for each triangle
		fileSTL.seek(84 + nCountTriangles * 50 + 24 + 0);
		datastream >> x[1];
		fileSTL.seek(84 + nCountTriangles * 50 + 24 + 4);
		datastream >> y[1];
		fileSTL.seek(84 + nCountTriangles * 50 + 24 + 8);
		datastream >> z[1];

		// x[2], y[2], z[2]: x, y, z coordinate for each triangle
		fileSTL.seek(84 + nCountTriangles * 50 + 36 + 0);
		datastream >> x[2];
		fileSTL.seek(84 + nCountTriangles * 50 + 36 + 4);
		datastream >> y[2];
		fileSTL.seek(84 + nCountTriangles * 50 + 36 + 8);
		datastream >> z[2];

		fileSTL.seek(84 + nCountTriangles * 50 + 48);
		datastream >> nControlBytes;

		// min/max calculation for all vertices
		for (int i = 0; i < 3; i++) {
			//Min
			if (x[i] < bb_min_.x)
				bb_min_.x = x[i];

			if (y[i] < bb_min_.y)
				bb_min_.y = y[i];

			if (z[i] < bb_min_.z)
				bb_min_.z = z[i];

			//Max
			if (x[i] > bb_max_.x)
				bb_max_.x = x[i];

			if (y[i] > bb_max_.y)
				bb_max_.y = y[i];

			if (z[i] > bb_max_.z)
				bb_max_.z = z[i];
		}

		// add new data
		Triangle T = {
			glm::vec3(x[0], y[0], z[0]),
			glm::vec3(x[1], y[1], z[1]),
			glm::vec3(x[2], y[2], z[2])
		};
		triForZorder.push_back(std::pair<Triangle, int>(T, nCountTriangles));
		vFaceNormals.push_back(glm::vec3(nx, ny, nz));
		nCountTriangles++;
	}
	fileSTL.close();

	return true;
	}

bool CW3Implant::implantLoadforPreviewWIM(const QString& manufacturer_name,
								   const QString& product_name,
								   const QString& flie_name,
								   glm::vec3 &NormFactor) {
	implant_path_ = flie_name;
	manufacturer_name_ = manufacturer_name;
	product_name_ = product_name;

	clock_t startTime = clock();

	QFile fileSTL(flie_name);
	QFileInfo fiSTL(flie_name);

	printf("implantLoad : %s\r\n", fiSTL.absoluteFilePath().toStdString().c_str());

	if (!fileSTL.open(QIODevice::ReadOnly)) {
#ifndef WILL3D_VIEWER
		QString err_msg = QString("%1 : %2").arg(implant_path_).arg(lang::LanguagePack::msg_01());
		CW3MessageBox msg_box("Will3D", err_msg, CW3MessageBox::Critical);
		msg_box.exec();
#endif
		return false;

	}

	fileSTL.seek(0);
	QDataStream datastream(&fileSTL);
	datastream.setByteOrder(QDataStream::LittleEndian);
	datastream.setFloatingPointPrecision(QDataStream::SinglePrecision);

	quint32 nTriangles;
	QString uuid;
	datastream >> uuid;
	datastream >> nTriangles;

	std::vector<glm::vec3>	face_normals;
	face_normals.reserve(nTriangles);
	std::vector<std::pair<Triangle, int>> triForZorder;
	triForZorder.reserve(nTriangles);
	int nCountTriangles = 0;
	while (nCountTriangles < nTriangles) {
		float nx, ny, nz, x[3], y[3], z[3];

		datastream >> x[0] >> y[0] >> z[0]
			>> x[1] >> y[1] >> z[1]
			>> x[2] >> y[2] >> z[2]
			>> nx >> ny >> nz;

		// add new data
		Triangle T = {
			glm::vec3(x[0], y[0], z[0]),
			glm::vec3(x[1], y[1], z[1]),
			glm::vec3(x[2], y[2], z[2])
		};
		triForZorder.push_back(std::pair<Triangle, int>(T, nCountTriangles));
		face_normals.push_back(glm::vec3(nx, ny, nz));
		++nCountTriangles;
	}
	fileSTL.close();

	// rotate direction
	if (manufacturer_name_.compare("DIO", Qt::CaseInsensitive) == 0) 
	{
		QStringList listFilePath = flie_name.split('/');
		QString fileName = listFilePath.at(listFilePath.size() - 1);
		if (product_name_.compare("UF II", Qt::CaseInsensitive) == 0 && fileName.toLower().contains("n"))
		{
			m_rotateDirection = glm::mat3(glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		}
		else
		{
			m_rotateDirection = glm::mat3(1.0f);
		}
	} 
	else if (manufacturer_name_.compare("MEGAGEN", Qt::CaseInsensitive) == 0) 
	{
		if (product_name.contains("ST", Qt::CaseInsensitive))
		{
			m_rotateDirection = glm::mat3(glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		}
		else
		{
			m_rotateDirection = glm::mat3(1.0f);
		}
	} 
	else if (manufacturer_name_.compare("Neobiotech", Qt::CaseInsensitive) == 0 ||
		manufacturer_name_.compare("NeoImplant", Qt::CaseInsensitive) == 0 ||
		manufacturer_name_.compare("DENTIS", Qt::CaseInsensitive) == 0 ||
		manufacturer_name_.compare("Shinhung", Qt::CaseInsensitive) == 0 ||
		manufacturer_name_.compare("Uris", Qt::CaseInsensitive) == 0 ||
		manufacturer_name_.compare("Biotem Implant", Qt::CaseInsensitive) == 0 ||
		manufacturer_name_.compare("Warantec", Qt::CaseInsensitive) == 0 || 
    manufacturer_name_.compare("ZimVie", Qt::CaseInsensitive) == 0)
	{
		m_rotateDirection = glm::mat3(glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	}
	else if (manufacturer_name_.compare("IBS", Qt::CaseInsensitive) == 0 ||
		manufacturer_name_.compare("IBS Implant", Qt::CaseInsensitive) == 0)
	{
		QString implant_name = QFileInfo(flie_name).fileName();
    QString implant_path = QFileInfo(flie_name).filePath();
		if (implant_name.contains("M3", Qt::CaseInsensitive) || implant_path.contains("/Magicore/", Qt::CaseInsensitive))
		{
			m_rotateDirection = glm::mat3(1.0f);
		}
		else
		{
			m_rotateDirection = glm::mat3(glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
		}
	} 
	else if (manufacturer_name_.compare("Osstem", Qt::CaseInsensitive) == 0) 
	{
		m_rotateDirection = glm::mat3(glm::rotate(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	}
	else if (manufacturer_name_.compare("Straumann", Qt::CaseInsensitive) == 0)
	{
		m_rotateDirection = glm::mat3(1.0f);
	}

	////sort rotate direction.
	glm::vec3 rotDir = m_rotateDirection * glm::vec3(0.0f, 0.0f, 1.0f);
	std::sort(triForZorder.begin(), triForZorder.end(),
			  [&](const std::pair<Triangle, int>& T1, const std::pair<Triangle, int>& T2) -> bool {
		float d1 = glm::dot(rotDir, (T1.first.v0 + T1.first.v1 + T1.first.v2) / 3.0f);
		float d2 = glm::dot(rotDir, (T2.first.v0 + T2.first.v1 + T2.first.v2) / 3.0f);
		return d1 < d2;
	}
	);

	vert_list_.clear();
	vert_list_.reserve(triForZorder.size() * 3);
	index_list_.clear();
	index_list_.reserve(triForZorder.size() * 3);
	std::map<glm::vec3, std::vector<int>, Vec3Less> mapVertIdx;
	std::vector<glm::vec3> zNorm;
	zNorm.reserve(triForZorder.size() * 3);
	for (int i = 0; i < triForZorder.size(); i++) {
		const Triangle& T = triForZorder[i].first;
		int idx = triForZorder[i].second;
		vert_list_.push_back(T.v0);
		vert_list_.push_back(T.v1);
		vert_list_.push_back(T.v2);

		const glm::vec3& n = face_normals[idx];
		zNorm.push_back(n);
		zNorm.push_back(n);
		zNorm.push_back(n);

		int idx_0 = i * 3;
		int idx_1 = idx_0 + 1;
		int idx_2 = idx_0 + 2;
		mapVertIdx[vert_list_[idx_0]].push_back(idx_0);
		mapVertIdx[vert_list_[idx_1]].push_back(idx_1);
		mapVertIdx[vert_list_[idx_2]].push_back(idx_2);
		index_list_.push_back(idx_0);
		index_list_.push_back(idx_1);
		index_list_.push_back(idx_2);
	}

	float maxFloat = std::numeric_limits<float>::max();
	float minFloat = std::numeric_limits<float>::min();
	bb_min_ = glm::vec3(maxFloat, maxFloat, maxFloat);
	bb_max_ = glm::vec3(minFloat, minFloat, minFloat);

	normal_list_.clear();
	normal_list_.reserve(vert_list_.size());
	for (int i = 0; i < vert_list_.size(); i++) {
		const glm::vec3& vert = vert_list_[i];
		// min/max calculation for all vertices
		// Min
		if (vert.x < bb_min_.x)
			bb_min_.x = vert.x;

		if (vert.y < bb_min_.y)
			bb_min_.y = vert.y;

		if (vert.z < bb_min_.z)
			bb_min_.z = vert.z;

		// Max
		if (vert.x > bb_max_.x)
			bb_max_.x = vert.x;

		if (vert.y > bb_max_.y)
			bb_max_.y = vert.y;

		if (vert.z > bb_max_.z)
			bb_max_.z = vert.z;

		glm::vec3 platNormal = zNorm[i];
		const auto& mapIter = mapVertIdx.find(vert_list_[i]);
		if (mapIter != mapVertIdx.end()) {
			const std::vector<int>& indices = mapIter->second;
			glm::vec3 normal(0.0f);
			for (const auto& index : indices) {
				glm::vec3 smoothNormal = zNorm[index];
				if (glm::length(smoothNormal - platNormal) < 0.35f)
					normal += smoothNormal;
			}

			normal_list_.push_back(glm::normalize(normal));
		}
	}

	glm::vec3 implantRange = bb_max_ - bb_min_;
	glm::vec3 centerPos = (bb_max_ + bb_min_) * 0.5f;
	glm::vec3 implantToNormGL = (1.0f / implantRange) * (2.0f / NormFactor);
	scaleToScaledGL(implantToNormGL, centerPos);

	m_maxLengthInGL = std::max(m_diameter, m_length);

	/*if (m_bUseOBB)
		makeOBBtree();*/

		//setVBO();

	float elapsedTime = static_cast<float>(clock() - startTime);
	std::cout << "IMPLANT load (ms): " << (elapsedTime) << std::endl;
	return true;
}

void CW3Implant::setVBO() {
	clearGLresources();

	m_pgGLWidget->makeCurrent();

	glGenBuffers(3, m_vbo);

	int N = vert_list_.size();

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, N * 3 * sizeof(float), vert_list_.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, N * 3 * sizeof(float), normal_list_.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_list_.size() * sizeof(unsigned int),
				 index_list_.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_pgGLWidget->doneCurrent();
}

void CW3Implant::scaleToScaledGL(glm::vec3 &NormFactor, glm::vec3 &Center) {
	float maxFloat = std::numeric_limits<float>::max();
	float minFloat = std::numeric_limits<float>::min();

	bb_min_ = glm::vec3(maxFloat, maxFloat, maxFloat);
	bb_max_ = glm::vec3(minFloat, minFloat, minFloat);

	glm::vec3 resize = glm::vec3(m_diameter, m_diameter, m_length);

	for (int i = 0; i < vert_list_.size(); i++) {
		vert_list_.at(i) = (vert_list_.at(i) - Center) * NormFactor;
		//m_vVertexNormals.at(i) = glm::normalize(m_vVertexNormals.at(i)*NormFactor);

		// rotate direction
		vert_list_.at(i) = vert_list_.at(i) * m_rotateDirection;
		normal_list_.at(i) = normal_list_.at(i) * m_rotateDirection;

		// resize
		vert_list_.at(i) = vert_list_.at(i) * resize;

		//Min
		if (vert_list_.at(i).x < bb_min_.x)
			bb_min_.x = vert_list_.at(i).x;

		if (vert_list_.at(i).y < bb_min_.y)
			bb_min_.y = vert_list_.at(i).y;

		if (vert_list_.at(i).z < bb_min_.z)
			bb_min_.z = vert_list_.at(i).z;

		//Max
		if (vert_list_.at(i).x > bb_max_.x)
			bb_max_.x = vert_list_.at(i).x;

		if (vert_list_.at(i).y > bb_max_.y)
			bb_max_.y = vert_list_.at(i).y;

		if (vert_list_.at(i).z > bb_max_.z)
			bb_max_.z = vert_list_.at(i).z;
	}
}

void CW3Implant::setModelToTextureMatrix(glm::vec3 &volRange) {
	m_ModelToTexture = m_GLtoTex * glm::scale(1.0f / volRange)*m_model;
	m_ForNormal = glm::transpose(glm::inverse(m_ModelToTexture));
}

void CW3Implant::setModel() {
	m_model = m_translate * m_rotate*m_scaleMatrix;
}
