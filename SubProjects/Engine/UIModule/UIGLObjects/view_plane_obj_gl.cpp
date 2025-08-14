#include "view_plane_obj_gl.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/plane_resource.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"

ViewPlaneObjGL::ViewPlaneObjGL()
	: GLObject(GL_TRIANGLES) 
{

}

ViewPlaneObjGL::~ViewPlaneObjGL() 
{

}

void ViewPlaneObjGL::Initialize() 
{
	const PlaneResource& resPlane = ResourceContainer::GetInstance()->GetPlaneResource();
	const std::vector<glm::vec3>& verts = resPlane.vertices();
	
	std::vector<glm::vec3> scaled_vertes;
	scaled_vertes.reserve(verts.size());
	
	for (const auto elem : verts)
	{
		scaled_vertes.push_back(elem * ver_tex_scale_);
	}

	const auto& indices = resPlane.indices();
	const auto& tex_coords = resPlane.tex_coord();
	GLObject::InitVAOVBOtexture(scaled_vertes, tex_coords, indices);
}
