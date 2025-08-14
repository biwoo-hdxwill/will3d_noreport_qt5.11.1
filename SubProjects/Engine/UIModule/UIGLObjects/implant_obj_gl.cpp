#include "implant_obj_gl.h"

#include <QDebug>

#include "../../Common/Common/W3Logger.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/implant_resource.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Common/Common/W3Logger.h"

ImplantObjGL::ImplantObjGL(int implant_id)
	: id_(implant_id), GLObject(GL_TRIANGLES) {
}

ImplantObjGL::~ImplantObjGL() {
}

void ImplantObjGL::Initialize() {
	const ImplantData& implant_data = GetImplantDataFromResource();

	const std::vector<glm::vec3>& verts = implant_data.mesh_vertices();
	const std::vector<glm::vec3>& norms = implant_data.mesh_normals();
	const std::vector<uint>& indices = implant_data.mesh_indices();

	this->InitVAOVBOmesh(verts, norms, indices);

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("ImplantObjGL::Initialize");
#endif 

}

void ImplantObjGL::GetLengthAndDiameter(float* diameter, float* length) const {
	const ImplantData& implant_data = GetImplantDataFromResource();
	*diameter = implant_data.diameter();
	*length = implant_data.length();
}
void ImplantObjGL::GetBoundingBoxRange(glm::vec3* max, glm::vec3* min) const{
	const ImplantData& implant_data = GetImplantDataFromResource();
	*max = implant_data.bounding_box_max();
	*min = implant_data.bounding_box_min();
}

glm::mat4 ImplantObjGL::GetTranslate(UIGLObjects::ObjCoordSysType type) const {

	const ImplantData& implant_data = GetImplantDataFromResource();
	switch (type) {
	case UIGLObjects::TYPE_VOLUME:
		return implant_data.translate_in_vol();
	case UIGLObjects::TYPE_PANORAMA:
		return implant_data.translate_in_pano();
	case UIGLObjects::TYPE_PANORAMA_SLICE:
		return implant_data.translate_in_pano_plane();
	default:
		assert(false);
		return glm::mat4(1.0f);
	}
	
}

glm::mat4 ImplantObjGL::GetRotate(UIGLObjects::ObjCoordSysType type) const {

	const ImplantData& implant_data = GetImplantDataFromResource();
	switch (type) {
	case UIGLObjects::TYPE_VOLUME:
		return implant_data.rotate_in_vol();
	case UIGLObjects::TYPE_PANORAMA:
		return implant_data.rotate_in_pano();
	case UIGLObjects::TYPE_PANORAMA_SLICE:
		return implant_data.rotate_in_pano_plane();
	default:
		assert(false);
		return glm::mat4(1.0f);
	}
}

const bool& ImplantObjGL::IsCollided() const {
	const ImplantData& implant_data = GetImplantDataFromResource();
	return implant_data.is_collide();
}

bool ImplantObjGL::IsSelected() const {
	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	int selected_id = res_implant.selected_implant_id();
	if (selected_id > 0 && id_ == selected_id)
		return true;
	else
		return false;

}

bool ImplantObjGL::IsValid() const {
	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	const auto& implant_datas = res_implant.data();
	auto iter = implant_datas.find(id_);

	if (iter == implant_datas.end()) {
		return false;
	} else return true;
}

const ImplantData& ImplantObjGL::GetImplantDataFromResource() const {
	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	const auto& implant_datas = res_implant.data();
	auto iter = implant_datas.find(id_);

	if (iter == implant_datas.end()) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR,
					  "ImplantObjGL::GetImplantDataFromResource: invalid id.");
		assert(false);
	}

	return *(iter->second.get());
}
