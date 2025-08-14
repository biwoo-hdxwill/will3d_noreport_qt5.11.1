#include "surface_items.h"

#include <assert.h>
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../UIGLObjects/gl_nerve_widget.h"
#include "../../UIGLObjects/gl_implant_widget.h"
#include "../../UIGLObjects/W3SurfaceItem.h"

#include "base_transform.h"

using namespace UIGLObjects;

SurfaceItems::SurfaceItems() {}
SurfaceItems::~SurfaceItems() {}

void SurfaceItems::InitItem(const ItemType& type, const UIGLObjects::ObjCoordSysType& coord_system_type) {
	switch (type) {
	case ItemType::NERVE:
		nerve_.reset(new GLNerveWidget(coord_system_type));
		break;
	case ItemType::IMPLANT:
		implant_.reset(new GLImplantWidget(coord_system_type));
		break;
	case ItemType::FACE:
		break;
	case ItemType::AIRWAY:
		break;
	default:
		assert(false);
		break;
	}
}

void SurfaceItems::SetVisible(const ItemType& type, bool is_visible) {
	if (nerve_)
		nerve_->set_is_visible(is_visible);
	if (implant_)
		implant_->set_is_visible(is_visible);
}

void SurfaceItems::SetSurfaceMVP(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p)
{
	if (nerve_) {
		nerve_->set_projection(p);
		nerve_->set_view(v);
		nerve_->set_world(m);
	}

	if (implant_) {
		implant_->set_projection(p);
		implant_->set_view(v);
		implant_->set_world(m);
	}
}

void SurfaceItems::SetSurfaceMVP(const BaseTransform& transform) {
	if (nerve_) {
		nerve_->set_projection(transform.projection());
		nerve_->set_view(transform.view());
		nerve_->set_world(transform.model());
		nerve_->SetTransformMat(transform.rotate(), ARCBALL);
		nerve_->SetTransformMat(transform.reorien(), REORIENTATION);
	}

	if (implant_) {
		implant_->set_projection(transform.projection());
		implant_->set_view(transform.view());
		implant_->set_world(transform.model());
		implant_->SetTransformMat(transform.rotate(), ARCBALL);
		implant_->SetTransformMat(transform.reorien(), REORIENTATION);
	}

	if (face_) {
		//face_->set_projection(transform.projection());
		//face_->set_view(transform.view());
		//face_->SetTransformMat(transform.rotate(), ARCBALL);
		//face_->SetTransformMat(transform.reorien(), REORIENTATION);
	}
}

void SurfaceItems::SetSurfaceTransformMat(const ItemType & type, const glm::mat4& mat,
										  const UIGLObjects::TransformType& transf_type) {
	switch (type) {
	case ItemType::NERVE:
		nerve_->SetTransformMat(mat, transf_type);
		break;
	case ItemType::IMPLANT:
		implant_->SetTransformMat(mat, transf_type);
		break;
	case ItemType::FACE:
		break;
	case ItemType::AIRWAY:
		break;
	default:
		assert(false);
		break;
	}
}

void SurfaceItems::ClearGL() {
	if (nerve_)
		nerve_->ClearVAOVBO();
	if (implant_)
		implant_->ClearVAOVBO();
	if (face_)
		face_->clearVAOVBO();
}

void SurfaceItems::RenderAll(const unsigned int& program) {
  CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");
	if (nerve_ && nerve_->is_visible())
	{
		glUseProgram(program);
		nerve_->Render(program);
	}

	CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");
	if (implant_ && implant_->is_visible())
	{
		glUseProgram(program);
		implant_->Render(program);
	}

	if (face_) {
		//face_->set_projection(transform.projection());
		//face_->set_view(transform.view());
		//face_->SetTransformMat(transform.rotate(), ARCBALL);
		//face_->SetTransformMat(transform.reorien(), REORIENTATION);
	}
}
void SurfaceItems::Render(const ItemType& type, const unsigned int& program) {
	switch (type) {
	case ItemType::NERVE:
		glUseProgram(program);
		nerve_->Render(program);
		break;
	case ItemType::IMPLANT:
		glUseProgram(program);
		implant_->Render(program);
		break;
	case ItemType::FACE:
		break;
	case ItemType::AIRWAY:
		break;
	default:
		assert(false);
		break;
	}
}
void SurfaceItems::RenderForPick(const ItemType& type, const unsigned int& program) {
	switch (type) {
	case ItemType::NERVE:
		assert(false);
		//glUseProgram(program);
		//nerve_->Render(program);
		break;
	case ItemType::IMPLANT:
		glUseProgram(program);
		implant_->RenderForPick(program);
		break;
	case ItemType::FACE:
		assert(false);
		break;
	case ItemType::AIRWAY:
		assert(false);
		break;
	default:
		assert(false);
		break;
	}
}
glm::mat4 SurfaceItems::GetSurfaceTransformMat(const ItemType & type,
											   const UIGLObjects::TransformType& transf_type) {
	switch (type) {
	case ItemType::NERVE:
		return nerve_->GetTransformMat(transf_type);
	case ItemType::IMPLANT:
		return implant_->GetTransformMat(transf_type);
	case ItemType::FACE:
		return mat4(1.0f);
	case ItemType::AIRWAY:
		return mat4(1.0f);
	default:
		assert(false);
		return mat4(1.0f);
	}
}

void SurfaceItems::InitNerve(const UIGLObjects::ObjCoordSysType& coord_system_type) {
	if (nerve_) {
		return PrintLogAndAssert("SurfaceItems::InitNerve: Already set.");
	}

	nerve_.reset(new GLNerveWidget(coord_system_type));
}

void SurfaceItems::InitImplant(const UIGLObjects::ObjCoordSysType& coord_system_type) {
	if (implant_) {
		return PrintLogAndAssert("SurfaceItems::InitImplant: Already set.");
	}

	implant_.reset(new GLImplantWidget(coord_system_type));
}

void SurfaceItems::InitFace() {}

void SurfaceItems::InitAirway() {}

void SurfaceItems::PrintLogAndAssert(const char* msg) {
	common::Logger::instance()->Print(common::LogType::WRN, msg);
	assert(false);
}

void SurfaceItems::ApplyPreferences()
{
	implant_->ApplyPreferences();
}
