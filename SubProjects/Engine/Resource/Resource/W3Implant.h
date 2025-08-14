#pragma once
/*=========================================================================

File:			class CW3Implant
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-05-20
Last date:		2016-06-02

=========================================================================*/
#if defined(__APPLE__)
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#include <GL/glew.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#endif

#include <QOpenGLWidget>

#include "W3Resource.h"
#include "resource_global.h"

class RESOURCE_EXPORT CW3Implant : public CW3Resource
{
public:
	CW3Implant(QOpenGLWidget *GLWidget, int id, float diameter, float length, bool useOBB = true);
	~CW3Implant();

	bool implantLoad(const QString& manufacturerName,
					 const QString& productName,
					 const QString& stlFileName);
	bool implantLoadforPreviewWIM(const QString& manufacturer_name,
						   const QString& product_name,
						   const QString& flie_name,
						   glm::vec3 &NormFactor);	// NormFactor: volume size (mm 단위)
	void setVBO();

	inline unsigned int* getVBO() { return m_vbo; }
	inline unsigned int getNindices() { return index_list_.size(); }

	inline float getMaxAxisLength() const noexcept { return m_maxLengthInGL; }

	void setModelToTextureMatrix(glm::vec3 &volRange);
	void setModel();

	QString getFilePath() { return implant_path_; }
	QString getManufacturerName() { return manufacturer_name_; }
	QString getProductName() { return product_name_; }
	inline const float length() const noexcept { return m_length; }
	inline const float diameter() const noexcept { return m_diameter; }

private:
	void init();
	void clearGLresources();
	
	void scaleToScaledGL(glm::vec3 &NormFactor, glm::vec3 &Center);

public:
	glm::mat4 m_scaleMatrix;

	glm::mat4 m_model;
	glm::mat4 m_rotate;
	glm::mat4 m_translate;

	glm::mat4 m_ModelToTexture;
	glm::mat4 m_ForNormal;

	glm::mat4 m_ModelToTextureForShading;
	glm::mat4 m_ForNormalShading;

	glm::mat4 m_mvp;
	glm::mat4 m_inverseScaledModel;

private:
	QOpenGLWidget *m_pgGLWidget;

	unsigned int m_vbo[3];

	std::vector<glm::vec3> vert_list_; //Vertex list
	std::vector<glm::vec3> normal_list_;
	std::vector<unsigned int> index_list_;// index list

	int m_id;
	float m_length; // 임플란트 길이, mm 단위
	float m_diameter; // 임플란트 지름, mm 단위
	QString implant_path_;
	QString manufacturer_name_;
	QString product_name_;

	glm::vec3 bb_min_; // 경계박스의 min값, scaled GL coordinate
	glm::vec3 bb_max_; // 경계박스의 max값, scaled GL coordinate

	glm::vec3 m_UpVector; // 임플란트 upvector
	glm::vec3 m_BackVector; // 임프란트 backvector
	glm::vec3 m_RightVector; // 임플란트 rightvector

	glm::mat4 m_GLtoTex;

	float m_maxLengthInGL;

	glm::mat3 m_rotateDirection;
};
