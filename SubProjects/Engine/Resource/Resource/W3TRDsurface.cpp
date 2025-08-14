#include "W3TRDsurface.h"
/*=========================================================================

File:		class CW3TRDsurface
Language:	C++11
Library:	Qt 5.4.1, Standard C++ Library
Author:			Hong Jung
First date:		2016-04-15
Last modify:	2016-04-15

=========================================================================*/
#include <QFile>
#include <QDebug>
//#include <QTextStream>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "W3Image3D.h"

using common::Logger;
using common::LogType;

CW3TRDsurface::CW3TRDsurface(QOpenGLWidget *GLWidget)
	: m_pgGLWidget(GLWidget)
{
	m_Npoints = 0;
	m_Nindices = 0;
	m_nsTex = 0;
	m_ntTex = 0;

	m_pPoints = nullptr;
	m_pNormals = nullptr;
	m_pTexCoord = nullptr;
	m_pIndices = nullptr;
	m_pTexImage = nullptr;

	// modified TRD
	m_NpointsOrg = 0;
	m_NindicesOrg = 0;

	m_pPointsOrg = nullptr;
	m_pNormalsOrg = nullptr;
	m_pTexCoordOrg = nullptr;
	m_pIndicesOrg = nullptr;

	m_pSRtoVol = glm::mat4(1.0f);

	m_bIsTexture = false;
	m_bIsOriginalTRD = true;

	m_vboSR[0] = 0;
	m_vboSR[1] = 0;
	m_vboSR[2] = 0;
	m_vboSR[3] = 0;

	m_vboSROrg[0] = 0;
	m_vboSROrg[1] = 0;

	m_texFACEHandler = 0;
}

CW3TRDsurface::~CW3TRDsurface()
{
	if (m_pgGLWidget->context())
	{
		m_pgGLWidget->makeCurrent();
		if (m_vboSR[0])
		{
			if (m_bIsTexture)
			{
				glDeleteBuffers(4, m_vboSR);
			}
			else
			{
				glDeleteBuffers(3, m_vboSR);
			}

			m_vboSR[0] = 0;
			m_vboSR[1] = 0;
			m_vboSR[2] = 0;
			m_vboSR[3] = 0;
		}

		if (m_texFACEHandler)
		{
			glDeleteTextures(1, &m_texFACEHandler);
			m_texFACEHandler = 0;
		}

		if (m_vboSROrg[0])
		{
			glDeleteBuffers(2, m_vboSROrg);
			m_vboSROrg[0] = 0;
			m_vboSROrg[1] = 0;
		}

		m_pgGLWidget->doneCurrent();
	}

	if (m_bIsTexture)
	{
		SAFE_DELETE_ARRAY(m_pPoints);
		SAFE_DELETE_ARRAY(m_pNormals);
		SAFE_DELETE_ARRAY(m_pIndices);
		SAFE_DELETE_ARRAY(m_pTexCoord);
		SAFE_DELETE_ARRAY(m_pTexImage);

		SAFE_DELETE_ARRAY(m_pPointsOrg);
		SAFE_DELETE_ARRAY(m_pNormalsOrg);
		SAFE_DELETE_ARRAY(m_pIndicesOrg);
		SAFE_DELETE_ARRAY(m_pTexCoordOrg);
	}
}

void CW3TRDsurface::trdLoad(const QString *filename, CW3Image3D *vol)
{
	std::string strLocal8Bit = filename->toLocal8Bit().toStdString();

	FILE *FREAD = nullptr;
	bool result = false;
#if defined(__APPLE__)
	FREAD = fopen(strLocal8Bit.c_str(), "rb");
	result = (FREAD) ? true : false;
#else
	result = !fopen_s(&FREAD, strLocal8Bit.c_str(), "rb");
#endif
	if (!result)
	{
		Logger::instance()->Print(LogType::ERR, "3D Photo File open is failed");
		return;
	}

	// 추가할 것
	// 1. 원본인지 변경된 trd 인지 확인할 bool
	// 2. 원본의 points 갯수
	// 3. 원본의 indices 갯수
	// 4. 원본의 points 데이터
	// 5. 원본의 normals 데이터
	// 6. 원본의 indices 데이터
	// * 점 데이터가 추가/삭제 되면 textrue coord 도 변경되어야 하나?

	fread(&m_Npoints, sizeof(unsigned int), 1, FREAD);
	fread(&m_Nindices, sizeof(unsigned int), 1, FREAD);
	fread(&m_nsTex, sizeof(unsigned int), 1, FREAD);
	fread(&m_ntTex, sizeof(unsigned int), 1, FREAD);

	qDebug() << "m_Npoints :" << m_Npoints;
	qDebug() << "m_Nindices :" << m_Nindices;
	qDebug() << "m_nsTex :" << m_nsTex;
	qDebug() << "m_ntTex :" << m_ntTex;

	W3::p_allocate_1D(&m_pPoints, m_Npoints);
	W3::p_allocate_1D(&m_pNormals, m_Npoints);
	W3::p_allocate_1D(&m_pTexCoord, m_Npoints);
	W3::p_allocate_1D(&m_pIndices, m_Nindices);
	W3::p_allocate_1D(&m_pTexImage, m_nsTex * m_ntTex * 3);

	fread(m_pPoints, sizeof(glm::vec3), m_Npoints, FREAD);
	fread(m_pNormals, sizeof(glm::vec3), m_Npoints, FREAD);
	fread(m_pTexCoord, sizeof(glm::vec2), m_Npoints, FREAD);
	fread(m_pIndices, sizeof(unsigned int), m_Nindices, FREAD);
	fread(m_pTexImage, sizeof(unsigned char), m_nsTex * m_ntTex * 3, FREAD);

#if 0 // convert ply to trd test
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> normals;
	unsigned int *index = nullptr;

	//QFile trdply("D:\\Work\\[00]3DProject\\SampleDCM\\model\\new_model.ply");
	/*if (trdply.open(QIODevice::ReadOnly))
	{
		std::cerr << "trdply open failed" << std::endl;
		return;
	}*/

	QFile ply("D:\\Work\\[00]3DProject\\SampleDCM\\model\\new_model_test.ply");
	if (!ply.open(QIODevice::ReadOnly))
	{
		std::cerr << "ply open failed" << std::endl;
		return;
	}

	QTextStream read(&ply);

	QString line;
	int numVertex = 0;
	int numFace = 0;
	int numIndex = 0;

	while (true)
	{
		line = read.readLine();

		if (line.contains("element vertex", Qt::CaseInsensitive) != 0)
		{
			numVertex = line.split(" ").at(2).toInt();
		}
		else if (line.contains("element face", Qt::CaseInsensitive) != 0)
		{
			numFace = line.split(" ").at(2).toInt();
		}
		else if (line.compare("end_header", Qt::CaseInsensitive) == 0)
		{
			break;
		}
	}

	numIndex = numFace * 3;
	W3::p_allocate_1D(&index, numIndex);

	float x, y, z, nx, ny, nz;
	int r, g, b;

	for (int i = 0; i < numVertex; i++)
	{
		read >> x >> y >> z;
		points.push_back(glm::vec3(x, y, z));

		read >> nx >> ny >> nz;
		normals.push_back(glm::vec3(nx, ny, nz));

		read >> r >> g >> b;
	}

	for (int i = 0; i < numFace; i++)
	{
		int number = 0;
		read >> number;

		int idx1 = 0;
		read >> idx1;
		index[i * 3] = idx1;

		int idx2 = 0;
		read >> idx2;
		index[i * 3 + 1] = idx2;

		int idx3 = 0;
		read >> idx3;
		index[i * 3 + 2] = idx3;
	}

	//trdply.close();
	ply.close();

	FILE *FWRITE;

	if (fopen_s(&FWRITE, "D:\\Work\\[00]3DProject\\SampleDCM\\model\\new_model_test.trd", "wb"))
	{
		std::cerr << "trd open failed" << std::endl;
		return;
	}

	fwrite(&numVertex, sizeof(unsigned int), 1, FWRITE);
	fwrite(&numIndex, sizeof(unsigned int), 1, FWRITE);
	fwrite(&m_nsTex, sizeof(unsigned int), 1, FWRITE);
	fwrite(&m_ntTex, sizeof(unsigned int), 1, FWRITE);

#if 1
	fwrite(&points[0], sizeof(glm::vec3), numVertex, FWRITE);
	fwrite(&normals[0], sizeof(glm::vec3), numVertex, FWRITE);
	fwrite(m_pTexCoord, sizeof(glm::vec2), numVertex, FWRITE);
	fwrite(index, sizeof(unsigned int), numIndex, FWRITE);
#else
	fwrite(m_pPoints, sizeof(glm::vec3), m_Npoints, FWRITE);
	fwrite(m_pNormals, sizeof(glm::vec3), m_Npoints, FWRITE);
	fwrite(m_pTexCoord, sizeof(glm::vec2), m_Npoints, FWRITE);
	fwrite(m_pIndices, sizeof(unsigned int), m_Nindices, FWRITE);
#endif
	fwrite(m_pTexImage, sizeof(unsigned char), m_nsTex * m_ntTex * 3, FWRITE);

	fclose(FWRITE);
#endif

	int rc = fgetc(FREAD);
	if (rc == EOF) // end of file 이거나 error 일 때 rc = EOF
	{
		m_bIsOriginalTRD = true; // end of file 이면 원본 TRD

		if (feof(FREAD))
		{ // 0 이 아니면 end of file
			Logger::instance()->Print(LogType::INF, "********** Original TRD **********");
		}
		else
		{
			Logger::instance()->Print(LogType::ERR, "CW3TRDsurface::trdLoad: error");
		}
	}
	else // trd 파일을 저장한 경우 rc는 무조건 1
	{
		Logger::instance()->Print(LogType::INF, "********** Modified TRD **********");
		ungetc(rc, FREAD);
		// 실제로 flag 값을 확인할 일이 없음
		// 원본이면 EOF 이고, 수정본이면 EOF가 아닌 것으로 판별 가능
		int flag = 0;
		fread(&flag, sizeof(int), 1, FREAD);
		m_bIsOriginalTRD = !flag ? true : false; // flag 가 0 이면 원본 TRD
	}

	if (!m_bIsOriginalTRD)
	{
		fread(&m_NpointsOrg, sizeof(unsigned int), 1, FREAD);
		fread(&m_NindicesOrg, sizeof(unsigned int), 1, FREAD);

		W3::p_allocate_1D(&m_pPointsOrg, m_NpointsOrg);
		//W3::p_allocate_1D(&m_pNormalsOrg, m_NpointsOrg);
		//W3::p_allocate_1D(&m_pTexCoordOrg, m_NpointsOrg);
		W3::p_allocate_1D(&m_pIndicesOrg, m_NindicesOrg);

		fread(m_pPointsOrg, sizeof(glm::vec3), m_NpointsOrg, FREAD);
		//fread(m_pNormalsOrg, sizeof(glm::vec3), m_NpointsOrg, FREAD);
		//fread(m_pTexCoordOrg, sizeof(glm::vec2), m_NpointsOrg, FREAD);
		fread(m_pIndicesOrg, sizeof(unsigned int), m_NindicesOrg, FREAD);
	}
	fclose(FREAD);

	//ConvertToSTL("trd.stl");
	//ConvertToPLY("trd.ply");
	//ConvertToOBJ("trd");

	float maxX = std::numeric_limits<float>::min();
	float minX = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();
	float minY = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::min();
	float minZ = std::numeric_limits<float>::max();

	for (int i = 0; i < m_Npoints; i++)
	{
		if (maxX < m_pPoints[i].x)
			maxX = m_pPoints[i].x;
		if (maxY < m_pPoints[i].y)
			maxY = m_pPoints[i].y;
		if (maxZ < m_pPoints[i].z)
			maxZ = m_pPoints[i].z;

		if (minX > m_pPoints[i].x)
			minX = m_pPoints[i].x;
		if (minY > m_pPoints[i].y)
			minY = m_pPoints[i].y;
		if (minZ > m_pPoints[i].z)
			minZ = m_pPoints[i].z;
	}

	std::string strLogMsg =
		"TRD Length X : " + std::to_string(maxX - minX) +
		", Y : " + std::to_string(maxY - minY) +
		", Z : " + std::to_string(maxZ - minZ);
	Logger::instance()->Print(LogType::INF, strLogMsg);

	float longestAxis = 1.0f;
	float xlength, ylength, zlength;
	if (vol)
	{
		xlength = (vol->width()) * 0.5f * vol->pixelSpacing();
		ylength = (vol->height()) * 0.5f * vol->pixelSpacing();
		zlength = (vol->depth()) * 0.5f * vol->sliceSpacing();
	}
	else
	{
		xlength = fabs(maxX - minX);
		ylength = fabs(maxY - minY);
		zlength = fabs(maxZ - minZ);

		longestAxis = std::max(std::max(xlength, ylength), zlength);

		xlength = longestAxis;
		ylength = longestAxis;
		zlength = longestAxis;

		Logger::instance()->Print(LogType::INF,
			std::string("longestAxis :") + std::to_string(longestAxis));
	}

	for (int i = 0; i < m_Npoints; i++)
	{
		m_pPoints[i].x /= xlength;
		m_pPoints[i].y /= ylength;
		m_pPoints[i].z /= zlength;
	}

	if (!m_bIsOriginalTRD)
	{
		for (int i = 0; i < m_NpointsOrg; i++)
		{
			m_pPointsOrg[i].x /= xlength;
			m_pPointsOrg[i].y /= ylength;
			m_pPointsOrg[i].z /= zlength;
		}
	}

	m_bIsTexture = true;
	//setVBO();
}

void CW3TRDsurface::ConvertToSTL(const QString& out_path)
{
	QFile fileSTL(out_path);
	fileSTL.open(QIODevice::WriteOnly);
	fileSTL.seek(0);

	QDataStream datastream(&fileSTL);

	datastream.setByteOrder(QDataStream::LittleEndian);
	datastream.setFloatingPointPrecision(QDataStream::SinglePrecision);

	quint32 nTriangles = m_Nindices / 3;
	quint16 nControlBytes = 0;

	fileSTL.seek(80);

	datastream << nTriangles;
	for (int i = 0; i < nTriangles; i++)
	{
		float nx, ny, nz, x[3], y[3], z[3];

		nx = (m_pNormals[m_pIndices[i * 3]].x + m_pNormals[m_pIndices[i * 3 + 1]].x + m_pNormals[m_pIndices[i * 3 + 2]].x) / 3;
		ny = (m_pNormals[m_pIndices[i * 3]].y + m_pNormals[m_pIndices[i * 3 + 1]].y + m_pNormals[m_pIndices[i * 3 + 2]].y) / 3;
		nz = (m_pNormals[m_pIndices[i * 3]].z + m_pNormals[m_pIndices[i * 3 + 1]].z + m_pNormals[m_pIndices[i * 3 + 2]].z) / 3;

		x[0] = m_pPoints[m_pIndices[i * 3]].x;
		y[0] = m_pPoints[m_pIndices[i * 3]].y;
		z[0] = m_pPoints[m_pIndices[i * 3]].z;

		x[1] = m_pPoints[m_pIndices[i * 3 + 1]].x;
		y[1] = m_pPoints[m_pIndices[i * 3 + 1]].y;
		z[1] = m_pPoints[m_pIndices[i * 3 + 1]].z;

		x[2] = m_pPoints[m_pIndices[i * 3 + 2]].x;
		y[2] = m_pPoints[m_pIndices[i * 3 + 2]].y;
		z[2] = m_pPoints[m_pIndices[i * 3 + 2]].z;

		// nx, ny, nz: normal for face
		fileSTL.seek(84 + i * 50 + 0 + 0);
		datastream << nx;
		fileSTL.seek(84 + i * 50 + 0 + 4);
		datastream << ny;
		fileSTL.seek(84 + i * 50 + 0 + 8);
		datastream << nz;

		// x[0], y[0], z[0]: x, y, z coordinate for each triangle
		fileSTL.seek(84 + i * 50 + 12 + 0);
		datastream << x[0];
		fileSTL.seek(84 + i * 50 + 12 + 4);
		datastream << y[0];
		fileSTL.seek(84 + i * 50 + 12 + 8);
		datastream << z[0];

		// x[1], y[1], z[1]: x, y, z coordinate for each triangle
		fileSTL.seek(84 + i * 50 + 24 + 0);
		datastream << x[1];
		fileSTL.seek(84 + i * 50 + 24 + 4);
		datastream << y[1];
		fileSTL.seek(84 + i * 50 + 24 + 8);
		datastream << z[1];

		// x[2], y[2], z[2]: x, y, z coordinate for each triangle
		fileSTL.seek(84 + i * 50 + 36 + 0);
		datastream << x[2];
		fileSTL.seek(84 + i * 50 + 36 + 4);
		datastream << y[2];
		fileSTL.seek(84 + i * 50 + 36 + 8);
		datastream << z[2];

		fileSTL.seek(84 + i * 50 + 48);
		datastream << nControlBytes;
	}
	fileSTL.close();
}

void CW3TRDsurface::ConvertToPLY(const QString& out_path)
{
	//QImage img(m_pTexImage, m_nsTex, m_ntTex, QImage::Format_RGB888);
	QImage img(m_nsTex, m_ntTex, QImage::Format_RGB888);
	for (int i = 0; i < m_ntTex; i++)
	{
		for (int j = 0; j < m_nsTex; j++)
		{
			int index = ((i * m_nsTex) + j) * 3;
			unsigned char b = m_pTexImage[index];
			unsigned char g = m_pTexImage[index + 1];
			unsigned char r = m_pTexImage[index + 2];
			QRgb color = qRgb(m_pTexImage[index], m_pTexImage[index + 1], m_pTexImage[index + 2]);
			img.setPixel(j, i, color);
		}
	}
	//bool rc2 = img.save("c:\\face.png", "PNG");
	printf("==================================================================== face texture : s %d, t %d\r\n", m_nsTex, m_ntTex);

	QFile ply(out_path);
	ply.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	int numTriangle = m_Nindices / 3;

	QTextStream out(&ply);
	out << "ply\n"
		<< "format ascii 1.0\n"
		<< "comment author: HDXWILL\n"
		<< "comment object: face\n"
		<< "element vertex " << m_Npoints << "\n"
		<< "property float x\n"
		<< "property float y\n"
		<< "property float z\n"
		<< "property uchar red\n"
		<< "property uchar green\n"
		<< "property uchar blue\n"
		<< "element face " << numTriangle << "\n"
		<< "property list uchar int vertex_index\n"
		<< "end_header\n";

	for (int i = 0; i < m_Npoints; i++)
	{
		QRgb rgb = img.pixel(m_pTexCoord[i].x * m_nsTex, m_pTexCoord[i].y * m_ntTex);
		out << m_pPoints[i].x << " " << m_pPoints[i].y << " " << m_pPoints[i].z << " " << qRed(rgb) << " " << qGreen(rgb) << " " << qBlue(rgb) << "\n";
	}

	for (int i = 0; i < numTriangle; i++)
	{
		out << "3 " << m_pIndices[i * 3] << " " << m_pIndices[i * 3 + 1] << " " << m_pIndices[i * 3 + 2] << "\n";
	}

	ply.close();
}

void CW3TRDsurface::ConvertToOBJ(const QString& out_file_name)
{
	bool include_texture = true;

	QString obj_path = out_file_name + ".obj";
	QString texture_path = out_file_name + ".png";
	QString mtl_path = out_file_name + ".mtl";
	QString mtl_name = "Face";

	// make .obj
	QFile obj(obj_path);
	obj.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	QTextStream obj_out(&obj);
	obj_out << "# HDXWILL Will3D v1.2.0\n";
	obj_out << "mtllib " << mtl_path << "\n";

	for (int i = 0; i < m_Npoints; i++)
	{
		obj_out << "v " <<
			m_pPoints[i].x << " " << 
			m_pPoints[i].y << " " << 
			m_pPoints[i].z << "\n";
	}
	obj_out << "\n";

	if (include_texture)
	{
		for (int i = 0; i < m_Npoints; i++)
		{
			obj_out << "vt " <<
				m_pTexCoord[i].x << " " <<
				-m_pTexCoord[i].y << "\n";
		}
		obj_out << "\n";
	}

	for (int i = 0; i < m_Npoints; i++)
	{
		glm::vec3 normalized_normal = glm::normalize(m_pNormals[i]);
		obj_out << "vn " <<
			normalized_normal.x << " " << 
			normalized_normal.y << " " << 
			normalized_normal.z << "\n";
	}
	obj_out << "\n";

	obj_out << "usemtl " << mtl_name << "\n";
	obj_out << "s off\n";

	int number_of_triangles = m_Nindices / 3;
	for (int i = 0; i < number_of_triangles; ++i)
	{
		int triangle_point_1 = m_pIndices[i * 3] + 1;
		int triangle_point_2 = m_pIndices[i * 3 + 1] + 1;
		int triangle_point_3 = m_pIndices[i * 3 + 2] + 1;

		if (include_texture)
		{
			obj_out << "f " <<
				triangle_point_1 << "/" << triangle_point_1 << "/" << triangle_point_1 << " " <<
				triangle_point_2 << "/" << triangle_point_2 << "/" << triangle_point_2 << " " <<
				triangle_point_3 << "/" << triangle_point_3 << "/" << triangle_point_3 << "\n";
		}
		else
		{
			obj_out << "f " <<
				triangle_point_1 << "/" << "/" << triangle_point_1 << " " <<
				triangle_point_2 << "/" << "/" << triangle_point_2 << " " <<
				triangle_point_3 << "/" << "/" << triangle_point_3 << "\n";
		}
	}

	obj.close();

	// make texture
	QImage texture(m_nsTex, m_ntTex, QImage::Format_RGB888);
	for (int i = 0; i < m_ntTex; i++)
	{
		for (int j = 0; j < m_nsTex; j++)
		{
			int index = ((i * m_nsTex) + j) * 3;
			unsigned char b = m_pTexImage[index];
			unsigned char g = m_pTexImage[index + 1];
			unsigned char r = m_pTexImage[index + 2];
			QRgb color = qRgb(m_pTexImage[index], m_pTexImage[index + 1], m_pTexImage[index + 2]);
			texture.setPixel(j, i, color);
		}
	}
	texture.save(texture_path, "PNG");

	// make .mtl
	QFile mtl(mtl_path);
	mtl.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	QTextStream mtl_out(&mtl);
	mtl_out << "# HDXWILL Will3D v1.2.0\n";

	mtl_out << "newmtl " << mtl_name << "\n";
	mtl_out << "Ns 100.0" << "\n";
	mtl_out << "Ni 1.0" << "\n";
	mtl_out << "Ka 1.0 1.0 1.0" << "\n";
	mtl_out << "Kd 1.0 1.0 1.0" << "\n";
	mtl_out << "Ks 0.5 0.5 0.5" << "\n";
	mtl_out << "d 1.0" << "\n";
	mtl_out << "illum 2" << "\n";
	mtl_out << "map_Kd " << texture_path << "\n";

	mtl.close();
}

void CW3TRDsurface::setVBO()
{
	m_pgGLWidget->makeCurrent();

	if (m_bIsTexture)
	{
		CW3GLFunctions::initVBO(m_vboSR, (float *)m_pPoints, (float *)m_pTexCoord, (float *)m_pNormals, m_Npoints, m_pIndices, m_Nindices);
		CW3GLFunctions::init2DTexSR(m_texFACEHandler, m_nsTex, m_ntTex, m_pTexImage);

		if (!m_bIsOriginalTRD)
		{
			//CW3GLFunctions::initVBOpointOnly(m_vboSROrg, (float *)m_pPointsOrg, m_NpointsOrg, m_pIndicesOrg, m_NindicesOrg); // W3SurfaceRenderingCL에서 vertex, index 만 필요한데 이렇게 하면 정상동작 하지 않음, 분석 필요.
			CW3GLFunctions::initVBO(m_vboSROrg, (float *)m_pPointsOrg, (float *)m_pTexCoord, (float *)m_pNormals, m_NpointsOrg, m_pIndicesOrg, m_NindicesOrg); // ok
		}
	}
	else
	{
		CW3GLFunctions::initVBOpointOnly(m_vboSR, (float*)m_pPoints, m_Npoints * 3, m_pIndices, m_Nindices);
		//CW3GLFunctions::initVBO(m_vboSR, (float*)m_pPoints, (float *)m_pNormals, m_Npoints * 3, m_pIndices, m_Nindices);
	}

	m_pgGLWidget->doneCurrent();
}

bool CW3TRDsurface::loadPassiveSurface(const std::vector<glm::vec3>& points,
	const std::vector<glm::vec3>& normals,
	const std::vector<unsigned int> indices,
	const std::vector<glm::vec2> texCoords,
	const QImage& image)
{
	if (points.size() == 0 ||
		normals.size() == 0 ||
		indices.size() == 0 ||
		texCoords.size() == 0)
	{
		Logger::instance()->Print(LogType::INF,
			std::string("CW3TRDsurface::loadPassiveSurface : input parameter, zero size."));
		return false;
	}

	if ((points.size() != normals.size()) ||
		(points.size() != texCoords.size()) ||
		(normals.size() != texCoords.size()))
	{
		Logger::instance()->Print(LogType::INF,
			std::string("CW3TRDsurface::loadPassiveSurface : parameter is not matching."));
		return false;
	}

	SAFE_DELETE_ARRAY(m_pPoints);
	SAFE_DELETE_ARRAY(m_pNormals);
	SAFE_DELETE_ARRAY(m_pTexCoord);
	SAFE_DELETE_ARRAY(m_pIndices);
	SAFE_DELETE_ARRAY(m_pTexImage);

	m_Npoints = points.size();
	m_Nindices = indices.size();

	m_pPoints = new glm::vec3[m_Npoints];
	memcpy(m_pPoints, &points[0], sizeof(glm::vec3)*m_Npoints);

	m_pNormals = new glm::vec3[m_Npoints];
	memcpy(m_pNormals, &normals[0], sizeof(glm::vec3)*m_Npoints);

	m_pIndices = new unsigned int[m_Nindices];
	memcpy(m_pIndices, &indices[0], sizeof(unsigned int)*m_Nindices);

	m_pTexCoord = new glm::vec2[m_Npoints];
	memcpy(m_pTexCoord, &texCoords[0], sizeof(glm::vec2)*m_Npoints);

	m_nsTex = image.width();
	m_ntTex = image.height();

	m_pTexImage = new unsigned char[m_nsTex*m_ntTex * 3];
	const unsigned char* data = image.bits();

	for (int i = 0, k = 0; i < m_nsTex*m_ntTex * 4; i += 4, k += 3)
	{
		m_pTexImage[k] = data[i];
		m_pTexImage[k + 1] = data[i + 1];
		m_pTexImage[k + 2] = data[i + 2];
	}

	m_bIsTexture = true;

	this->setVBO();
	return true;
}

std::vector<glm::vec3> CW3TRDsurface::getPoints()
{
	std::vector<glm::vec3> points;
	points.resize(m_Npoints);
	memcpy(&points[0], m_pPoints, sizeof(glm::vec3)*m_Npoints);
	return points;
}

std::vector<glm::vec3> CW3TRDsurface::getNormals()
{
	std::vector<glm::vec3> normals;
	normals.resize(m_Npoints);
	memcpy(&normals[0], m_pNormals, sizeof(glm::vec3)*m_Npoints);
	return normals;
}

std::vector<unsigned int> CW3TRDsurface::getIndices()
{
	std::vector<unsigned int> indices;
	indices.resize(m_Nindices);
	memcpy(&indices[0], m_pIndices, sizeof(unsigned int)*m_Nindices);

	return indices;
}

std::vector<glm::vec2> CW3TRDsurface::getTexCoord()
{
	std::vector<glm::vec2> texCoord;

	texCoord.resize(m_Npoints);
	memcpy(&texCoord[0], m_pTexCoord, sizeof(glm::vec2)*m_Npoints);

	return texCoord;
}
//thyoo end
// 
