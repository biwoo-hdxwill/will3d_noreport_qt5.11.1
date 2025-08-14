#pragma once
/*=========================================================================

File:		class CW3TRDsurface
Language:	C++11
Library:	Qt 5.4.1, Standard C++ Library
Author:			Hong Jung
First date:		2016-04-15
Last modify:	2016-04-15

=========================================================================*/
#include <GL/glew.h>
#include <qopenglwidget.h>

#include "resource_global.h"
#include "W3Resource.h"

#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#endif

class CW3Image3D;

class RESOURCE_EXPORT CW3TRDsurface : public CW3Resource {
public:
	CW3TRDsurface(QOpenGLWidget *GLWidget);
	~CW3TRDsurface();

public:
	bool loadPassiveSurface(
		const std::vector<glm::vec3>& points,
		const std::vector<glm::vec3>& normals,
		const std::vector<unsigned int> indices,
		const std::vector<glm::vec2> texCoords,
		const QImage& image);

	std::vector<glm::vec3> getPoints();
	std::vector<glm::vec3> getNormals();
	std::vector<unsigned int> getIndices();
	std::vector<glm::vec2> getTexCoord();

	inline int getTexWidth() { return m_nsTex; }
	inline int getTexHeight() { return m_ntTex; }
	inline unsigned char* getTexData() { return m_pTexImage; }
	void trdLoad(const QString *filename, CW3Image3D *vol = nullptr);
	void setVBO();

	inline unsigned int* getVBO() { return m_vboSR; }
	inline unsigned int* getVBOOrg() { return m_bIsOriginalTRD ? m_vboSR : m_vboSROrg; }
	inline unsigned int getNindices() { return m_Nindices; }
	inline unsigned int getTexHandler() { return m_texFACEHandler; }
	inline glm::mat4 getSRtoVol() { return m_pSRtoVol; }
	inline void setPoints(glm::vec3 *Points) { m_pPoints = Points; }
	inline void setNormals(glm::vec3 *Normals) { m_pNormals = Normals; }
	inline void setIndices(unsigned int *Indices) { m_pIndices = Indices; }
	inline void setNpoints(int N) { m_Npoints = N; }
	inline void setNindices(unsigned int N) { m_Nindices = N; }
	inline void setSRtoVol(const glm::mat4& model) { m_pSRtoVol = model; }

private:
	void ConvertToSTL(const QString& out_path);
	void ConvertToPLY(const QString& out_path);
	void ConvertToOBJ(const QString& out_file_name);

private:
	QOpenGLWidget	*m_pgGLWidget;

	unsigned int m_Npoints;
	unsigned int m_Nindices;
	unsigned int m_nsTex;
	unsigned int m_ntTex;

	glm::vec3 *m_pPoints;
	glm::vec3 *m_pNormals;
	glm::vec2 *m_pTexCoord;
	unsigned int *m_pIndices;
	unsigned char *m_pTexImage;

	// modified TRD
	unsigned int m_NpointsOrg;
	unsigned int m_NindicesOrg;

	glm::vec3	*m_pPointsOrg;
	glm::vec3	*m_pNormalsOrg;
	glm::vec2	*m_pTexCoordOrg;
	unsigned int *m_pIndicesOrg;

	unsigned int m_vboSR[4];
	unsigned int m_vboSROrg[4];
				 
	unsigned int m_texFACEHandler;

	glm::mat4	m_pSRtoVol;

	bool m_bIsTexture;
	bool m_bIsOriginalTRD;
};

