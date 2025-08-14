#pragma once
/*=========================================================================

File:			class ProjectIOVTOSTO
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-18
Last modify:	2016-07-18

=========================================================================*/
#include <memory>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif
#include <H5Cpp.h>

#include "../../Common/Common/W3Enum.h"
#include "datatypes.h"
#include "w3projectio_global.h"

namespace project {
typedef struct _VTOSTOFlags VTOSTOFlags;
}

class W3PROJECTIO_EXPORT ProjectIOVTOSTO {
public:
	ProjectIOVTOSTO(const project::Purpose& purpose,
					const std::shared_ptr<H5::H5File>& file);
	~ProjectIOVTOSTO();

	ProjectIOVTOSTO(const ProjectIOVTOSTO&) = delete;
	ProjectIOVTOSTO& operator=(const ProjectIOVTOSTO&) = delete;

public:
	void InitVTOSTO();
	bool IsInit();

	// save functions
	void SaveIsoValue(float iso_value);
	void SaveHeadPoints(const std::vector<glm::vec3>& head_points);
	void SaveHeadTriIndices(const std::vector<std::vector<int>>& tri_indices);

	void SaveFacePoints(const std::vector<glm::vec3>& face_points);
	void SaveFacePointsAfter(const std::vector<glm::vec3>& face_points_after);
	void SaveFaceIndices(const std::vector<unsigned int>& face_indices);
	void SaveFaceTexCoords(const std::vector<glm::vec2>& tex_coords);

	void SaveModelPoints(const std::vector<glm::vec3>& model_points);
	void SaveModelTetraIndices(const std::vector<std::vector<int>>& tetra_indices);
	void SaveModelTriIndices(const std::vector<std::vector<int>>& model_tri_indices);
	void SaveModelTetraMoveResult(const std::vector<glm::vec3>& move_results);
	void SaveModelPhotoToSurface(const glm::mat4& mat);

	void SaveVTOSTOFlags(const project::VTOSTOFlags& flags);

	// load functions
	void LoadIsoValue(float& iso_value);
	void LoadHeadPoints(std::vector<glm::vec3>& head_points);
	void LoadHeadTriIndices(std::vector<std::vector<int>>& tri_indices);

	void LoadFacePoints(std::vector<glm::vec3>& face_points);
	void LoadFacePointsAfter(std::vector<glm::vec3>& face_points_after);
	void LoadFaceIndices(std::vector<unsigned int>& face_indices);
	void LoadFaceTexCoords(std::vector<glm::vec2>& tex_coords);

	void LoadModelPoints(std::vector<glm::vec3>& model_points);
	void LoadModelTetraIndices(std::vector<std::vector<int>>& tetra_indices);
	void LoadModelTriIndices(std::vector<std::vector<int>>& model_tri_indices);
	void LoadModelTetraMoveResult(std::vector<glm::vec3>& move_result);
	void LoadModelPhotoToSurface(glm::mat4& mat);

	void LoadVTOSTOFlags(project::VTOSTOFlags& flags);

	// face trd load & save functions
	void SaveTRDPath(const std::string& path);
	void SaveTRDPoints(const glm::vec3* points, size_t count);
	void SaveTRDNormals(const glm::vec3* normals, size_t count);
	void SaveTRDTexCoords(const glm::vec2* tex_coords, size_t count);
	void SaveTRDIndices(const unsigned int* indices, size_t count);
	void SaveTRDTexImage(const unsigned char * image,
						 unsigned int tex_w, unsigned int tex_h);

	void LoadTRDPath(std::string& path);
	void LoadTRDPoints(std::vector<glm::vec3>& points);
	void LoadTRDNormals(std::vector<glm::vec3>& normals);
	void LoadTRDTexCoords(std::vector<glm::vec2>& tex_coords);
	void LoadTRDIndices(std::vector<unsigned int>& indices);
	void LoadTRDTexImage(std::vector<unsigned char>& image,
						 unsigned int& tex_w, unsigned int& tex_h);

private:
	std::shared_ptr<H5::H5File> file_;
	H5::CompType vtosto_flags_type_;
};

