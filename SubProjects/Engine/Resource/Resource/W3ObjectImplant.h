#pragma once
/*=========================================================================

File:			class CW3ObjectImplant
Language:		C++11
Library:		Qt 5.4.0
Author:			SANG KEUN PARK, Hong Jung
First date:		2015-12-10
Last date:		2016-05-20	

입력된 모든 Implant를 통합적으로 가지고 있는 class
=========================================================================*/
#include "resource_global.h"

#include <GL/glew.h>

#include <QObject>
#include <QVector3D>
#include <QQuaternion>
#include <QColor>
#include <QList>

#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>

#include "../../Common/Common/W3Matrix4x4.h"
#include "../../Common/Common/W3Vector3D.h"
#include "../../Common/Common/W3Vector2D.h"
#include "../../Common/Common/W3Point3D.h"
#include "../../Common/Common/W3Types.h"
#include <qopenglwidget.h>

struct MESH3D_new
{

	W3INT ID;//입력된 implant구분을 위한 index

	std::vector<glm::vec3> listVertices; //Vertex list
	std::vector<W3UINT> listIndices;// index list

	std::vector<glm::vec3> listVertexNormals;	// surface의 Normal vector
	//std::vector<glm::vec3> listFaceNormals;	// vertex의 Normal vector

	//QList<int> listNumVertices; //vertex 전체갯수


	glm::vec3 vecBoundMin_original; // 경계박스의 min값, 입력받은 stl 파일기준
	glm::vec3 vecBoundMax_original; // 경계박스의 max값, 입력받은 stl 파일기준

	glm::vec3 vecBoundMin_modify;// 경계박스의 min값, vecBoundMin_original 값을 입력받은 volume을 기준으로 scale 된 값
	glm::vec3 vecBoundMax_modify;// 경계박스의 max값, vecBoundMax_original 값을 입력받은 volume을 기준으로 scale 된 값

	glm::vec3 vecTranslation; // 임플란트 위치 정보

	glm::vec3 UpVector; // 임플란트 upvector
	glm::vec3 BackVector; // 임프란트 backvector
	glm::vec3 RightVector; // 임플란트 rightvector

	QColor color; // 임플란트 색상

	W3FLOAT length; // 임플란트 길이
	W3FLOAT diameter; // 임플란트 지름

	glm::mat4 mRotateMPR; // 임플란트 회전정보
	glm::mat4 mRotateVR; // 임프란트 회전정보.  3D Axis surface 때문에 작성. 이후 수정한 후 지워야함. 

	W3BOOL bCollide; // 임플란트 충돌여부 정보
};

class CW3ImplantData;

class RESOURCE_EXPORT CW3ObjectImplant : public QObject
{
	Q_OBJECT

public:
	explicit CW3ObjectImplant(QOpenGLWidget *GLWidget, QObject *parent=0);
	~CW3ObjectImplant();

public:
	// 임프란트 추가.
	W3INT Add(QString fileName, W3INT implantID, QColor color, glm::vec3 translation, W3FLOAT diameter, W3FLOAT length, glm::vec3 upVector, glm::vec3 backVector, glm::vec3 rightVector, glm::mat4 rotateMatrix);
	W3INT Add(QString fileName, W3INT implantID);


	void setSelectedImplant(W3INT idx);// 현재 선택된 임플란트 ID 저장
	W3BOOL setSelectedImplant(glm::vec3 position, glm::vec3 volRange);// 3차워 좌표를 입력받아 해당위치에 있는 임플란트 ID를 저장
	W3INT getSelectedImplant();// 선택된 임플란트 ID를 가져옴
	W3BOOL isImplantSelected(); // 임프란트가 선택된 상태인지 여부 확인
	void Delete(W3INT implantID);// 입력받은 ID의 임플란트 삭제
	void DeleteAll(); // 모든 임플란트 삭제
	void setColor(W3INT implantID, QColor color);// 입력받은 ID의 임플란트 색상 저장
	void setTranslation(W3INT implantID, glm::vec3 translation);// 입력받은 ID의 임플란트 위치정보 저장
	
	//void setShow(W3BOOL show);// 임플란트 visible 상태 저장
	//void setGeometry(W3FLOAT diameter, W3FLOAT length);//
	void setImplantMaxCout(W3INT count);// 등록 가능한 임플란트 갯수 저장

	//W3BOOL getShow();
	W3INT getImplantCount();//저장된 임플란트 갯수를 얻어옴.
	QList<CW3ImplantData *> getImplantList();// 임플란트들을 저장하고 있는 QList 멤버 변수를 가져옴
	QColor getColor(W3INT implantID);// 입력받은 ID의 임플란트의 색상정보는 가져옴.
	//MESH3D_new getImplantMesh(W3INT implantID);// 입력받은 ID의 임플란트의 MESH3D_new 구조체를 가져옴
	glm::vec3 getTranslation(W3INT implantID);// 입력받은 ID의 임플란트의 위치정보를 가져옴.
	W3FLOAT getDiameter(W3INT implantID);// 입력받은 ID의 임플란트의 지름 정보를 가져옴
	W3FLOAT getLength(W3INT implantID);// 입력받은 ID의 임플란트의 길이를 가져욤

	glm::vec3 getBoundMin(W3INT implantID);// 입력받은 ID의 임플란트의 경계박의 max값을 가져옴.vecBoundMin_original 값을 입력받은 volume을 기준으로 scale 된 값
	glm::vec3 getBoundMax(W3INT implantID);// 입력받은 ID의 임플란트 경계박스의 min값을 가져옴.vecBoundMax_original 값을 입력받은 volume을 기준으로 scale 된 값

	glm::vec3 getBoundMin_original(W3INT implantID);// 입력받은 ID의 임플란트의 경계박의 max값을 가져옴.입력받은 stl 파일기준
	glm::vec3 getBoundMax_original(W3INT implantID);// 입력받은 ID의 임플란트 경계박스의 min값을 가져옴.v입력받은 stl 파일기준

	void setBoundMin(W3INT implantID, glm::vec3 boundMIN_norm);// 입력받은 ID의 임플란트의 경계박의 min값을 저장.vecBoundMin_original 값을 입력받은 volume을 기준으로 scale 된 값
	void setBoundMax(W3INT implantID, glm::vec3 boundMAX_norm);// 입력받은 ID의 임플란트의 경계박의 max값을 저장vecBoundMax_original 값을 입력받은 volume을 기준으로 scale 된 값

	void setGeometry(W3INT implantID, W3FLOAT diameter, W3FLOAT length);// 입력받은 ID의 임플란트의 지름과 길이를 저장


	void setUpVector(W3INT implantID, glm::vec3 upVector);// 입력받은 ID의 임플란트의 upvector 저장
	void setBackVector(W3INT implantID, glm::vec3 backVector);// 입력받은 ID의 임플란트의 backvector 저장
	void setRightVector(W3INT implantID, glm::vec3 rightVector);// 입력받은 ID의 임플란트의 rightvector 저장

	glm::vec3 getUpVector(W3INT implantID);// 입력받은 ID의 임플란트의 upvector를 가져옴
	glm::vec3 getBackVector(W3INT implantID);// 입력받은 ID의 임플란트의 backvector를 가져옴
	glm::vec3 getRightVector(W3INT implantID);// 입력받은 ID의 임플란트의 rightvecgtor를 가져옴
	W3INT getImplantID(W3INT listIndex);// 입력받은 list index에 해당하는 임플란트 ID를 가져옴.
	

	
	void setRotateMatrixMPR(W3INT implantID, glm::mat4 rotateMatrix); // 입력받은 ID에 해당하는 임플란트의 회전정보를 저장
	void setRotateMatrixVR(W3INT implantID, glm::mat4 rotateMatrix);  // 입력받은 ID에 해당하는 임플란트의 회전정보를 저장. 3D Axis surface 문제로 임시로 사용

	glm::mat4 getRotateMatrixMPR(W3INT implantID);// 입력받은 ID에 해당하는 임플란트의 회정정보를 가져옴.
	glm::mat4 getRotateMatrixVR(W3INT implantID);// 입력받은 ID에 해당하는 임플란트의 회전정보를 가져옴.3D Axis surface 문제로 임시로 사용

	void setImplantID(W3INT ListIndex, W3INT implantID);// 입력받은 List index에 ID를 저장

	inline W3INT getImplantMaxCount() { return m_nImplantMaxCount; }// 저장될 수 있는 최대 임플란트 갯수를 가져옴.

	void setSelected(W3BOOL selected){ m_bSelected = selected; }// 임플란트가 선택된 상태 저장
	W3BOOL getSelected(){ return m_bSelected; }// 임플란트 선택여부를 가져옴.

	W3INT getListIndex(W3INT implantID);// 입력받은 ID에 해당하는 임플란트의 List index를 가져옴.

	void setCollide(W3INT implantID, W3BOOL isCollide);// 입력받은 ID에 해당하는 임플란트의 충동여부를 저장
	W3BOOL getCollide(W3INT implantID);// 입력받은 ID에 해당하는 임플란트의 충돌여부를 가져옴.
	
private:	
	QOpenGLWidget *m_pgGLWidget;
	
	QList<CW3ImplantData *> m_listImplant; // 임프란트들을 가지고있는 QList
	W3BOOL m_bImplantSelected; // 임플란트가 선택된 상태인지 여부 
	W3INT m_selectedImplantID;// 현재 선택된 임플란트 ID
	W3INT m_nImplantMaxCount; // 입력받을 수 있는 임플란트 최대 갯수
	W3BOOL m_bSelected; // 임플란트 선택여부 

	
	
};

/*=========================================================================

File:			class CW3ImplantData
Language:		C++11
Library:		Qt 5.4.0
Author:			SANG KEUN PARK, Hong Jung 
First date:		2015-12-10
Last date:		2016-05-20	

각 각의 Implant의 정보를 저장
=========================================================================*/
class CW3ImplantData : public QObject
{
	Q_OBJECT

public:
	explicit CW3ImplantData(QOpenGLWidget *GLWidget, QObject *parent = 0);
	~CW3ImplantData();	
	
	W3BOOL setImplantData_new(QString stlFileName); //STL 파일을 읽어 버텍스 정보 저장

	void setColor(QColor color); // 임플란트 색상저장
	void setTranslation(glm::vec3 translation); // 임플란트 위치 정보 저장
	void setGeometry(W3FLOAT diameter, W3FLOAT length); // 임플란트 지름, 길이정보 저장
	void setUpVector(glm::vec3 upVector); // 임플란트 upvector 저장
	void setBackVector(glm::vec3 BackVector); // 임플란트 backvector 저장
	void setRightVector(glm::vec3 RightVector);// 임플란트 rightVector 저장
	void setVectors(glm::vec3 upVector, glm::vec3 backVector, glm::vec3 rightVector); // 임플란트의 right, up, back vector 저장
	void setRotateMatrixMPR(glm::mat4 rotateMatrix); // 임플란트의 회전 Matrix 저장
	void setRotateMatrixVR(glm::mat4 rotateMatrix); // 임플란트의 회전 Matrix 저장.  3D Axis surface 때문에 작성. 이후 수정한 후 지워야함. 
	void setImplantID(W3INT idx);// 임플란트의 ID 저장
	void setBoundMax(glm::vec3 boundMAX_norm);// 임플란트 경계 박스의 max 값
	void setBoundMin(glm::vec3 boundMIN_norm);// 임프란트 경계 박스의 min 값
	void setCollide(W3BOOL isCollide);// 임플란트간 충동여부 저장

	//MESH3D_new getImplantMesh(); // 임프란트 정보를 가지고 있는 MESH3D_new 자료형의 변수 불러오기.

private:
	void clearGLresources();
	void setVBO();

public:
	glm::vec3 m_vecBoundMin_original; // 경계박스의 min값, 입력받은 stl 파일기준
	glm::vec3 m_vecBoundMax_original; // 경계박스의 max값, 입력받은 stl 파일기준

	glm::vec3 m_vecBoundMin_modify;// 경계박스의 min값, vecBoundMin_original 값을 입력받은 volume을 기준으로 scale 된 값
	glm::vec3 m_vecBoundMax_modify;// 경계박스의 max값, vecBoundMax_original 값을 입력받은 volume을 기준으로 scale 된 값

	glm::vec3 m_vecTranslation; // 임플란트 위치 정보

	glm::vec3 m_UpVector; // 임플란트 upvector
	glm::vec3 m_BackVector; // 임프란트 backvector
	glm::vec3 m_RightVector; // 임플란트 rightvector

	QColor m_color; // 임플란트 색상

	W3FLOAT m_length; // 임플란트 길이
	W3FLOAT m_diameter; // 임플란트 지름

	glm::mat4 m_RotateMPR; // 임플란트 회전정보
	glm::mat4 m_RotateVR; // 임프란트 회전정보.  3D Axis surface 때문에 작성. 이후 수정한 후 지워야함. 

	W3BOOL m_bCollide; // 임플란트 충돌여부 정보


private:
	QOpenGLWidget *m_pgGLWidget;

	W3UINT		m_vbo[3];

	W3INT m_ID;//입력된 implant구분을 위한 index

	std::vector<glm::vec3> m_vVertices; //Vertex list
	std::vector<glm::vec3> m_vVertexNormals;
	std::vector<W3UINT> m_vIndices;// index list
	
	

	//MESH3D_new m_mesh3d_new;// 임프란트의 정보를 가지고 있는 변수
};
