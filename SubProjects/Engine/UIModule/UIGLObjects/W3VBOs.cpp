#include "W3VBOs.h"

#include <qmath.h>

#include <QDebug>

#include <Engine/Common/Common/W3Logger.h>
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/io_stl.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

class Vec3Less {
public:
	bool operator()(const glm::vec3& v0, const glm::vec3& v1) const {
		return v0.x < v1.x ||
			(v0.x == v1.x && v0.y < v1.y) ||
			(v0.x == v1.x && v0.y == v1.y && v0.z < v1.z);
	}
};

CW3VBOSTL::CW3VBOSTL(const QString& stlPath) {
	IO_STL::readBinarySTLFile(stlPath, m_s3MeshData, false);
	
	InitVBO();
	initVAO();
}
CW3VBOSTL::~CW3VBOSTL() {
	clearVAOVBO();
}

void CW3VBOSTL::InitVBO()
{
	vertices_.clear();
	normals_.clear();
	indices_.clear();

	std::map<glm::vec3, std::vector<int>, Vec3Less> mapVertIdx;

	for (int i = 0; i < m_s3MeshData.listVertices.length(); i++)
	{
		vec3 v = vec3(m_s3MeshData.listVertices[i].x(), m_s3MeshData.listVertices[i].y(), m_s3MeshData.listVertices[i].z());

		mapVertIdx[v].push_back(i);
		vertices_.push_back(v);
	}

	int cnt = 0;
	for (int i = 0; i < vertices_.size(); i++)
	{
		auto mapIter = mapVertIdx.find(vertices_[i]);

		vec3 platNormal = vec3(m_s3MeshData.listNormals[i / 3].x(), m_s3MeshData.listNormals[i / 3].y(), m_s3MeshData.listNormals[i / 3].z());

		if (mapIter != mapVertIdx.end())
		{
			std::vector<int> indices = mapIter->second;
			vec3 normal = vec3(0.0f);
			for (const auto& index : indices)
			{
				vec3 smoothNormal = vec3(m_s3MeshData.listNormals[index / 3].x(), m_s3MeshData.listNormals[index / 3].y(), m_s3MeshData.listNormals[index / 3].z());
				if (glm::length(smoothNormal - platNormal) < 0.35f)
					normal += smoothNormal;
			}

			normals_.push_back(glm::normalize(normal));
			indices_.push_back(i);
		}
	}
}

void CW3VBOSTL::initVAO() 
{
	CW3GLFunctions::initVBO(vboHandle, vertices_, normals_, indices_);

	//// Create the VAO
	//CW3GLFunctions::InitVAOvn(&vaoHandle, vboHandle);
	CW3GLFunctions::initVAO(&vaoHandle, vboHandle);

	std::string log_msg = QString("initVAO vaoHandle : %1").arg(vaoHandle).toStdString();
	common::Logger::instance()->Print(common::LogType::INF, log_msg);
}

void CW3VBOSTL::clearVAOVBO() {
	if (vboHandle[0]) {
		glDeleteBuffers(3, vboHandle);
		vboHandle[0] = 0;
		vboHandle[1] = 0;
		vboHandle[2] = 0;
	}
	if (vaoHandle) {
		glDeleteVertexArrays(1, &vaoHandle);
		vaoHandle = 0;
	}
}

void CW3VBOSTL::render() {
	if (!vaoHandle)
		initVAO();

	//CW3GLFunctions::initVAO(&vaoHandle, vboHandle);

	glBindVertexArray(vaoHandle);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandle[2]);
	//glDrawArrays(GL_TRIANGLES, 0, m_s3MeshData.listVertices.length());
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, (GLuint*)NULL);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//qDebug() << "render vaoHandle :" << vaoHandle;
}

void CW3VBOSTL::quarterRender() {
	if (!vaoHandle)
		initVAO();

	//CW3GLFunctions::initVAO(&vaoHandle, vboHandle);

	glBindVertexArray(vaoHandle);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandle[2]);
	//glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_s3MeshData.listVertices.length()*0.25f));
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(static_cast<float>(indices_.size()) * 0.25f), GL_UNSIGNED_INT, (GLuint*)NULL);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//qDebug() << "quarterRender vaoHandle :" << vaoHandle;
}

int CW3VBOSTL::getVertexArrayHandle() {
	return this->vaoHandle;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//VBOSTL end
///////////////////////////////////////////////////////////////////////////////////////////////////

CW3VBOSphere::CW3VBOSphere() {
	vboHandle[0] = 0;
	vaoHandle = 0;

	m_radius = 0.0f;
	m_slices = 0;
	m_stacks = 0;
	m_position = vec3(0.0);
}

CW3VBOSphere::~CW3VBOSphere() {
	if (vboHandle[0]) {
		glDeleteBuffers(3, vboHandle);
		vboHandle[0] = 0;
		vboHandle[1] = 0;
		vboHandle[2] = 0;
	}
	if (vaoHandle) {
		glDeleteVertexArrays(1, &vaoHandle);
		vaoHandle = 0;
	}
}
void CW3VBOSphere::createSphere(float radius, int slices, int stacks, glm::vec3 position) {
	m_radius = radius;
	m_slices = slices;
	m_stacks = stacks;
	m_position = position;

	if (vboHandle[0]) {
		glDeleteBuffers(3, vboHandle);
		vboHandle[0] = 0;
		vboHandle[1] = 0;
		vboHandle[2] = 0;
	}

	if (vaoHandle) {
		glDeleteVertexArrays(1, &vaoHandle);
		vaoHandle = 0;
	}

	std::vector<vec3> vert, norm;
	indices.clear();

	for (int i = 0; i <= stacks; ++i) {
		// V texture coordinate.
		float V = i / (float)stacks;
		float phi = V * M_PI;

		for (int j = 0; j <= slices; ++j) {
			// U texture coordinate.
			float U = j / (float)slices;
			float theta = U * M_PI * 2.0f;

			float X = cos(theta) * sin(phi);
			float Y = cos(phi);
			float Z = sin(theta) * sin(phi);

			vert.push_back(vec3(X, Y, Z) * radius + vec3(position));
			norm.push_back(vec3(X, Y, Z));
			//textureCoords.push_back(vec2(U, V));
		}
	}

	for (int i = 0; i < slices * stacks + slices; ++i) {
		indices.push_back(i);
		indices.push_back(i + slices + 1);
		indices.push_back(i + slices);

		indices.push_back(i + slices + 1);
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	CW3GLFunctions::initVBO(vboHandle, vert, norm, indices);
	CW3GLFunctions::initVAO(&vaoHandle, vboHandle);
}
void CW3VBOSphere::clearVAOVBO() {
	if (vboHandle[0]) {
		glDeleteBuffers(3, vboHandle);
		vboHandle[0] = 0;
		vboHandle[1] = 0;
		vboHandle[2] = 0;
	}

	if (vaoHandle) {
		glDeleteVertexArrays(1, &vaoHandle);
		vaoHandle = 0;
	}

	indices.clear();
}
void CW3VBOSphere::render() {
	if (!vaoHandle && m_radius != 0.0f)
		this->createSphere(m_radius, m_slices, m_stacks, m_position);

	//CW3GLFunctions::initVAO(&vaoHandle, vboHandle);
	glBindVertexArray(vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandle[2]);
	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		indices.size(),    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//CW3VBOSphere end
//////////////////////////////////////////////////////////////////////////////////////////////////////
