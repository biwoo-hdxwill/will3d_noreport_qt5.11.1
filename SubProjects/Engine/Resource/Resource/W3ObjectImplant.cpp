/*=========================================================================

File:			class CW3ObjectImplant
Language:		C++11
Library:		Qt 5.4.0
Author:			SANG KEUN PARK, Hong Jung 
First date:		2015-12-10
Last date:		2016-05-20

입력된 모든 Implant를 통합적으로 가지고 있는 class
=========================================================================*/
#include "W3ObjectImplant.h"
#include <QFile>
#include <qdebug.h>
#include <ctime>
#include <memory>
#include <iostream>

class Vec3Less{
public:
	bool operator()(const glm::vec3& v0, const glm::vec3& v1){
		return v0.x < v1.x ||
			(v0.x == v1.x && v0.y < v1.y) ||
			(v0.x == v1.x && v0.y == v1.y && v0.z < v1.z);
	}
};


CW3ObjectImplant::CW3ObjectImplant(QOpenGLWidget *GLWidget, QObject *parent)
: QObject(parent), m_pgGLWidget(GLWidget)
{
	m_selectedImplantID = -1;
	m_nImplantMaxCount = 28;
	m_bSelected = false;
}

CW3ObjectImplant::~CW3ObjectImplant()
{
	for(auto iter=m_listImplant.begin(); iter != m_listImplant.end(); iter++)
	{		
		delete *iter;
		m_listImplant.erase(iter);
	}
}


W3INT CW3ObjectImplant::Add(QString fileName, W3INT implantIndex, QColor color, glm::vec3 translation, W3FLOAT diameter, W3FLOAT length, glm::vec3 upVector, glm::vec3 backVector, glm::vec3 rightVector, glm::mat4 rotateMatrix)
{
	CW3ImplantData * implantData = new CW3ImplantData(m_pgGLWidget);
	m_listImplant.push_back(implantData);

	W3INT listIndex = m_listImplant.length() - 1;
	m_selectedImplantID = implantIndex;

	m_listImplant[listIndex]->setImplantData_new(fileName);
	m_listImplant[listIndex]->setColor(color);
	m_listImplant[listIndex]->setTranslation(translation);
	m_listImplant[listIndex]->setGeometry(diameter, length);
	m_listImplant[listIndex]->setUpVector(upVector);
	m_listImplant[listIndex]->setBackVector(backVector);
	m_listImplant[listIndex]->setRightVector(rightVector);
	m_listImplant[listIndex]->setRotateMatrixMPR(rotateMatrix);
	m_listImplant[listIndex]->setRotateMatrixVR(rotateMatrix);
	m_listImplant[listIndex]->setImplantID(implantIndex);
	m_listImplant[listIndex]->setCollide(false);
	
	return m_selectedImplantID;
}

W3INT CW3ObjectImplant::Add(QString fileName, W3INT implantIndex)
{
	CW3ImplantData * implantData = new CW3ImplantData(m_pgGLWidget);
	
	m_selectedImplantID = implantIndex;

	glm::vec3 rightVector(1.0f, 0.0f, 0.0f);
	glm::vec3 backVector(0.0f, 1.0f, 0.0f);
	glm::vec3 upVector(0.0f, 0.0f, 1.0f);

	glm::mat4 rotateMatrix(1.0f);

	QColor color(255, 0, 0, 255);
	glm::vec3 translation(0.0, 0.0f, 50.0f);
	W3FLOAT diameter = 4.0f;
	W3FLOAT length = 11.0f;

	if (implantIndex < 14)
	{
		rotateMatrix = glm::rotate(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	implantData->setImplantData_new(fileName);
	implantData->setColor(color);
	implantData->setTranslation(translation);
	implantData->setGeometry(diameter, length);
	implantData->setUpVector(upVector);
	implantData->setBackVector(backVector);
	implantData->setRightVector(rightVector);
	implantData->setRotateMatrixMPR(rotateMatrix);
	implantData->setRotateMatrixVR(rotateMatrix);
	implantData->setImplantID(implantIndex);
	implantData->setCollide(false);

	m_listImplant.push_back(implantData);

	return m_selectedImplantID;
}

//Modify
bool CW3ObjectImplant::isImplantSelected()
{	
	if (m_selectedImplantID == -1)
	{
		return false;
	}else{
		return true;
	}
}

void CW3ObjectImplant::Delete(W3INT implantID)
{

	W3INT listIndex = getListIndex(implantID);

	auto iter = m_listImplant.at(listIndex);
	delete iter;

	m_listImplant.removeAt(listIndex);

	W3INT selectedListIndex = m_listImplant.length() - 1;
	m_selectedImplantID = getImplantID(selectedListIndex);	

}

void CW3ObjectImplant::DeleteAll()
{
	for(auto iter=m_listImplant.begin(); iter != m_listImplant.end(); iter++)
	{		
		delete *iter; 
		m_listImplant.erase(iter);
	}
}


W3INT  CW3ObjectImplant::getSelectedImplant()
{
	return m_selectedImplantID;
}


W3INT CW3ObjectImplant::getImplantCount()
{	
	return m_listImplant.size();
}

QList<CW3ImplantData *> CW3ObjectImplant::getImplantList()
{
	return m_listImplant;
}
QColor CW3ObjectImplant::getColor(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->m_color;
}

//MESH3D_new CW3ObjectImplant::getImplantMesh(W3INT implantID)
//{
//	W3INT listIndex = getListIndex(implantID);
//	
//	return m_listImplant[listIndex]->getImplantMesh();
//}

glm::vec3 CW3ObjectImplant::getTranslation(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->m_vecTranslation;
}


W3FLOAT CW3ObjectImplant::getDiameter(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().diameter;
}

W3FLOAT CW3ObjectImplant::getLength(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().length;
}

glm::vec3 CW3ObjectImplant::getBoundMin(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().vecBoundMin_modify;
}

glm::vec3 CW3ObjectImplant::getBoundMax(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().vecBoundMax_modify;
}

glm::vec3 CW3ObjectImplant::getBoundMin_original(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().vecBoundMin_original;
}

glm::vec3 CW3ObjectImplant::getBoundMax_original(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().vecBoundMax_original;
}


glm::vec3 CW3ObjectImplant::getUpVector(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().UpVector;
}
glm::vec3 CW3ObjectImplant::getBackVector(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().BackVector;
}

glm::vec3 CW3ObjectImplant::getRightVector(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	
	return m_listImplant[listIndex]->getImplantMesh().RightVector;
}

W3INT CW3ObjectImplant::getImplantID(W3INT listIndex)
{
	return m_listImplant[listIndex]->getImplantMesh().ID;
}

W3INT CW3ObjectImplant::getListIndex(W3INT implantID)
{
	
	for (W3INT i = 0; i < m_listImplant.size(); i++)
	{
		if (m_listImplant[i]->getImplantMesh().ID == implantID)
		{
			return i;
		}
	}
	return -1;
}

glm::mat4 CW3ObjectImplant::getRotateMatrixMPR(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	return m_listImplant[listIndex]->getImplantMesh().mRotateMPR;
}

glm::mat4 CW3ObjectImplant::getRotateMatrixVR(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	return m_listImplant[listIndex]->getImplantMesh().mRotateVR;
}

W3BOOL CW3ObjectImplant::getCollide(W3INT implantID)
{
	W3INT listIndex = getListIndex(implantID);
	return m_listImplant[listIndex]->getImplantMesh().bCollide;
}

////////////////////////////////////////////////////////////////////////////
//Setting function
////////////////////////////////////////////////////////////////////////////
//Modify
void CW3ObjectImplant::setSelectedImplant(W3INT implantID)
{
	m_selectedImplantID = implantID;
}

//Modify
//좌표를 입력받아 선택된 좌표에 해당하는 Implant index를 셋팅하는 함수
W3BOOL CW3ObjectImplant::setSelectedImplant(glm::vec3 position, glm::vec3 volRange)
{
	W3INT implantCount = m_listImplant.size();

	W3BOOL bInOut = true;

	for (W3INT i = 0; i < implantCount; i++)
	{

		bInOut = true;

		glm::vec3 boundMin = m_listImplant[i]->getImplantMesh().vecBoundMin_modify * volRange;
		glm::vec3 boundMax = m_listImplant[i]->getImplantMesh().vecBoundMax_modify * volRange;

		QList<glm::vec3> boundPoints;
		boundPoints.clear();

		glm::vec3 tempPoint;
		tempPoint = glm::vec3(boundMax.x, boundMax.y, boundMax.z);
		boundPoints.push_back(tempPoint);

		tempPoint = glm::vec3(boundMax.x, boundMax.y, boundMin.z);
		boundPoints.push_back(tempPoint);

		tempPoint = glm::vec3(boundMax.x, boundMin.y, boundMax.z);
		boundPoints.push_back(tempPoint);

		tempPoint = glm::vec3(boundMax.x, boundMin.y, boundMin.z);
		boundPoints.push_back(tempPoint);

		tempPoint = glm::vec3(boundMin.x, boundMax.y, boundMax.z);
		boundPoints.push_back(tempPoint);

		tempPoint = glm::vec3(boundMin.x, boundMax.y, boundMin.z);
		boundPoints.push_back(tempPoint);

		tempPoint = glm::vec3(boundMin.x, boundMin.y, boundMax.z);
		boundPoints.push_back(tempPoint);

		tempPoint = glm::vec3(boundMin.x, boundMin.y, boundMin.z);
		boundPoints.push_back(tempPoint);


		MESH3D_new mesh = m_listImplant[i]->getImplantMesh();
		glm::mat4 rot = mesh.mRotateVR;
		glm::mat4 trans = glm::translate(mesh.vecTranslation);

		for (W3INT j = 0; j < 8; j++)
		{
			boundPoints[j] = glm::vec3(trans * rot * glm::vec4(boundPoints[j], 1.0f));

		}

		//calculate Normal
		//1,2,3->0,1,2
		//1,3,5->0,2,4
		//1,5,2->0,4,1

		//8,2,4->7,1,3
		//8,4,7->7,3,6
		//8,7,5->7,6,4

		QList<glm::vec3> planes_normal;
		//QList<W3FLOAT> planes_trans;
		QList<glm::vec3> points_normal;

		planes_normal.clear();
		//planes_trans.clear();
		points_normal.clear();

		planes_normal.push_back(glm::normalize(glm::cross((boundPoints[2] - boundPoints[0]), (boundPoints[1] - boundPoints[0]))));
		planes_normal.push_back(glm::normalize(glm::cross((boundPoints[4] - boundPoints[0]), (boundPoints[2] - boundPoints[0]))));
		planes_normal.push_back(glm::normalize(glm::cross((boundPoints[1] - boundPoints[0]), (boundPoints[4] - boundPoints[0]))));

		planes_normal.push_back(glm::normalize(glm::cross((boundPoints[1] - boundPoints[7]), (boundPoints[3] - boundPoints[7]))));
		planes_normal.push_back(glm::normalize(glm::cross((boundPoints[3] - boundPoints[7]), (boundPoints[6] - boundPoints[7]))));
		planes_normal.push_back(glm::normalize(glm::cross((boundPoints[6] - boundPoints[7]), (boundPoints[4] - boundPoints[7]))));


		//planes_trans.push_back(glm::dot(boundPoints[0], planes_normal[0]));
		//planes_trans.push_back(glm::dot(boundPoints[0], planes_normal[1]));
		//planes_trans.push_back(glm::dot(boundPoints[0], planes_normal[2]));

		//planes_trans.push_back(glm::dot(boundPoints[7], planes_normal[3]));
		//planes_trans.push_back(glm::dot(boundPoints[7], planes_normal[4]));
		//planes_trans.push_back(glm::dot(boundPoints[7], planes_normal[5]));

		points_normal.push_back(glm::normalize(position - boundPoints[0]));
		points_normal.push_back(glm::normalize(position - boundPoints[0]));
		points_normal.push_back(glm::normalize(position - boundPoints[0]));

		points_normal.push_back(glm::normalize(position - boundPoints[7]));
		points_normal.push_back(glm::normalize(position - boundPoints[7]));
		points_normal.push_back(glm::normalize(position - boundPoints[7]));


		W3FLOAT rlt = 0.0f;		

		for (W3INT j = 0; j < 6; j++)
		{
			//rlt = glm::dot(position, planes_normal[j]) - planes_trans[j];
			rlt = glm::dot(points_normal[j], planes_normal[j]);

			if (rlt > 0)
			{
				bInOut = false;
			}
		}	

		if (bInOut == true)
		{
			m_selectedImplantID = getImplantID(i);
			return true;
		}

	}
	return false;
}

void CW3ObjectImplant::setUpVector(W3INT implantID, glm::vec3 upVector)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setUpVector(upVector);
}

void CW3ObjectImplant::setBackVector(W3INT implantID, glm::vec3 backVector)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setBackVector(backVector);
}

void CW3ObjectImplant::setRightVector(W3INT implantID, glm::vec3 rightVector)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setRightVector(rightVector);
}

void CW3ObjectImplant::setRotateMatrixMPR(W3INT implantID, glm::mat4 rotateMatrix)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setRotateMatrixMPR(rotateMatrix);
}

void CW3ObjectImplant::setRotateMatrixVR(W3INT implantID, glm::mat4 rotateMatrix)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setRotateMatrixVR(rotateMatrix);
}

void CW3ObjectImplant::setImplantID(W3INT ListIndex, W3INT implantID)
{
	m_listImplant[ListIndex]->setImplantID(implantID);
}


void CW3ObjectImplant::setImplantMaxCout(W3INT count)
{
	m_nImplantMaxCount = count;
}

void CW3ObjectImplant::setColor(W3INT implantIndex, QColor color)
{
	W3INT listIndex = getListIndex(implantIndex);
	m_listImplant[listIndex]->setColor(color);
}

void CW3ObjectImplant::setTranslation(W3INT implantID, glm::vec3 translation)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setTranslation(translation);
}


void CW3ObjectImplant::setBoundMin(W3INT implantID, glm::vec3 boundMIN_norm)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setBoundMin(boundMIN_norm);
}

void CW3ObjectImplant::setBoundMax(W3INT implantID, glm::vec3 boundMAX_norm)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setBoundMax(boundMAX_norm);
}

void CW3ObjectImplant::setGeometry(W3INT implantID, W3FLOAT diameter, W3FLOAT length)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setGeometry(diameter, length);
}

void CW3ObjectImplant::setCollide(W3INT implantID, W3BOOL isCollide)
{
	W3INT listIndex = getListIndex(implantID);
	m_listImplant[listIndex]->setCollide(isCollide);
}

/*=========================================================================

File:			class CW3ImplantData
Language:		C++11
Library:		Qt 5.2.0
Author:			SANG KEUN PARK 
First date:		2015-12-10
Last modify:	

각 각의 Implant의 정보를 저장
=========================================================================*/
CW3ImplantData::CW3ImplantData(QOpenGLWidget *GLWidget, QObject *parent)
: QObject(parent), m_pgGLWidget(GLWidget)
{
	m_vbo[0] = 0;
	m_vbo[1] = 0;
	m_vbo[2] = 0;
}
CW3ImplantData::~CW3ImplantData()
{
	clearGLresources();

}

void CW3ImplantData::clearGLresources()
{
	if (m_pgGLWidget->context())
	{
		m_pgGLWidget->makeCurrent();
		if (m_vbo[0])
		{
			glDeleteBuffers(3, m_vbo);
			m_vbo[0] = 0;
			m_vbo[1] = 0;
			m_vbo[2] = 0;
		}

		m_pgGLWidget->doneCurrent();
	}
}

bool CW3ImplantData::setImplantData_new(QString stlFileName)
{
	clock_t startTime = clock();

	m_vVertices.clear();
	m_vIndices.clear();
	m_vVertexNormals.clear();

	std::map<glm::vec3, W3UINT, Vec3Less> mapVertIdx;
	W3INT indexCount = 0;

	QFile fileSTL(stlFileName);

	if (!fileSTL.open(QIODevice::ReadOnly))
	{
		return false;
	}

	fileSTL.seek(0);

	QDataStream datastream(&fileSTL);

	datastream.setByteOrder(QDataStream::LittleEndian);
	datastream.setFloatingPointPrecision(QDataStream::SinglePrecision);

	quint32 nTriangles;
	quint16 nControlBytes;

	fileSTL.seek(80);
	datastream >> nTriangles;

	quint32 nCountTriangles = 0;

	m_vecBoundMin_original = glm::vec3(10000, 10000, 10000);
	m_vecBoundMax_original = glm::vec3(-10000, -10000, -10000);

	std::vector<W3UINT>	vCountSameVertex;
	std::vector<glm::vec3>	vFaceNormals;

	while (nCountTriangles < nTriangles)
	{
		W3FLOAT nx, ny, nz, x[3], y[3], z[3];
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
		fileSTL.seek(84 + nCountTriangles * 50 + 24 + 0);

		// x[1], y[1], z[1]: x, y, z coordinate for each triangle
		datastream >> x[1];
		fileSTL.seek(84 + nCountTriangles * 50 + 24 + 4);
		datastream >> y[1];
		fileSTL.seek(84 + nCountTriangles * 50 + 24 + 8);
		datastream >> z[1];
		fileSTL.seek(84 + nCountTriangles * 50 + 36 + 0);

		// x[2], y[2], z[2]: x, y, z coordinate for each triangle
		datastream >> x[2];
		fileSTL.seek(84 + nCountTriangles * 50 + 36 + 4);
		datastream >> y[2];
		fileSTL.seek(84 + nCountTriangles * 50 + 36 + 8);
		datastream >> z[2];

		fileSTL.seek(84 + nCountTriangles * 50 + 48);
		datastream >> nControlBytes;

		// min/max calculation for all vertices
		for (int i = 0; i<3; i++)
		{
			//Min
			if (x[i] < m_vecBoundMin_original.x)
				m_vecBoundMin_original.x = x[i];

			if (y[i] < m_vecBoundMin_original.y)
				m_vecBoundMin_original.y = y[i];

			if (z[i] < m_vecBoundMin_original.z)
				m_vecBoundMin_original.z = z[i];

			//Max
			if (x[i] > m_vecBoundMax_original.x)
				m_vecBoundMax_original.x = x[i];

			if (y[i] > m_vecBoundMax_original.y)
				m_vecBoundMax_original.y = y[i];

			if (z[i] > m_vecBoundMax_original.z)
				m_vecBoundMax_original.z = z[i];
		}

		// add new data

		glm::vec3 faceNormal(nx, ny, nz);

		for (W3INT i = 0; i < 3; i++)
		{
			glm::vec3 vertex = glm::vec3(x[i], y[i], z[i]);
			if (mapVertIdx.find(vertex) == mapVertIdx.end())
			{
				mapVertIdx[vertex] = indexCount;
				m_vVertices.push_back(vertex);
				m_vIndices.push_back(indexCount++);
				vFaceNormals.push_back(faceNormal);
				vCountSameVertex.push_back(1);
			}
			else
			{
				W3UINT id = mapVertIdx[vertex];
				m_vIndices.push_back(id);
				vFaceNormals.at(id) += faceNormal;
				vCountSameVertex.at(id)++; 
			}
		}
		nCountTriangles++;
	}	


	fileSTL.close();

	for (W3INT i = 0; i < vCountSameVertex.size(); i++)
	{
		glm::vec3 normal = vFaceNormals.at(i) / W3FLOAT(vCountSameVertex.at(i));

		normal = glm::normalize(normal);

		m_vVertexNormals.push_back(normal);
	}


	
	clock_t endTime = clock();
	float elapsedTime;
	elapsedTime = static_cast<float>(endTime - startTime);
	std::cout << "IMPLANT load (ms): " << (elapsedTime) << std::endl;

	setVBO();

	return true;
}

void CW3ImplantData::setVBO()
{
	clearGLresources();

	m_pgGLWidget->makeCurrent();

	
	glGenBuffers(3, m_vbo);

	W3INT N = m_vVertices.size();

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, N*3* sizeof(W3FLOAT), m_vVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, N*3* sizeof(W3FLOAT), m_vVertexNormals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vIndices.size() * sizeof(W3UINT), m_vIndices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_pgGLWidget->doneCurrent();
}


void CW3ImplantData::setColor(QColor color)
{
	m_color = color;
}

void CW3ImplantData::setTranslation(glm::vec3 translation)
{
	m_vecTranslation = translation;
}


void CW3ImplantData::setBoundMax(glm::vec3 boundMAX_norm)
{
	m_vecBoundMax_modify = boundMAX_norm;
}

void CW3ImplantData::setBoundMin(glm::vec3 boundMIN_norm)
{
	m_vecBoundMin_modify = boundMIN_norm;
}


void CW3ImplantData::setCollide(W3BOOL isCollide)
{
	m_bCollide = isCollide;
}




void CW3ImplantData::setGeometry(W3FLOAT diameter, W3FLOAT length)
{
	m_diameter = diameter;
	m_length = length;
}

//MESH3D_new CW3ImplantData::getImplantMesh()
//{
//	return mesh3d_new;
//}

void CW3ImplantData::setUpVector(glm::vec3 upVector)
{
	m_UpVector = upVector;
}

void CW3ImplantData::setBackVector(glm::vec3 backVector)
{
	m_BackVector = backVector;
}

void CW3ImplantData::setRightVector(glm::vec3 rightVector)
{
	m_RightVector = rightVector;
}

void CW3ImplantData::setVectors(glm::vec3 upVector, glm::vec3 backVector, glm::vec3 rightVector)
{
	m_UpVector = upVector;
	m_BackVector = backVector;
	m_RightVector = rightVector;
}

void CW3ImplantData::setRotateMatrixMPR(glm::mat4 rotateMatrix)
{
	m_RotateMPR = rotateMatrix;
}

void CW3ImplantData::setRotateMatrixVR(glm::mat4 rotateMatrix)
{
	m_RotateVR = rotateMatrix;
}

void CW3ImplantData::setImplantID(W3INT idx)
{
	m_ID = idx;
}
