#include "implant_collision_renderer.h"


#include <qstring.h>
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/event_handler.h"
#include "../../Common/Common/event_handle_common.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/Resource/nerve_resource.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../../UIModule/UIGLObjects/implant_obj_gl.h"

#include "collide.h"
#include "defines.h"

using namespace UIGLObjects;
namespace {
	const int kViewportSize = 1000;
}

ImplantCollisionRenderer::ImplantCollisionRenderer(QObject* parent) :QObject(parent), BaseOffscreenRenderer() {
	if (BaseOffscreenRenderer::MakeCurrent()) {
		WGLSLprogram::createShaderProgram(QString(":/surface/surface.vert"),
										  QString(":/surface/surface.frag"), prog_render_);

		BaseOffscreenRenderer::DoneCurrent();
	}
}

ImplantCollisionRenderer::~ImplantCollisionRenderer() {
	if (BaseOffscreenRenderer::MakeCurrent()) {
		glDeleteProgram(prog_render_);
		if (depth_buffer_) {
			glDeleteBuffers(1, &depth_buffer_);
			depth_buffer_ = 0;
		}
		BaseOffscreenRenderer::DoneCurrent();
	}
}

void ImplantCollisionRenderer::CheckCollide(const glm::mat4& projection_view, std::vector<int>* collided_ids, bool is_move_implant) 
{
	if (!is_connted_obj_gl_) 
	{
		ConnectObjGL();
	}

	if (MakeCurrent()) 
	{
		if (is_update_implant_) 
		{
			ClearImplantVAOVBO();
			is_update_implant_ = false;
		}

		if (is_update_nerve_) 
		{
			ClearNerveVAOVBO();
			is_update_nerve_ = false;
		}

		BindFrameBufferObject();
		if (!depth_buffer_)
		{
			glGenRenderbuffers(1, &depth_buffer_);
			glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, kViewportSize, kViewportSize);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
			ReleaseFrameBufferObject();
		}

		glViewport(0, 0, kViewportSize, kViewportSize);
		CW3GLFunctions::clearView(true);

		const auto& res_container = ResourceContainer::GetInstance();
		const CW3Image3D& vol = res_container->GetMainVolume();
		const ImplantResource& res_implant = res_container->GetImplantResource();
		const NerveResource& res_nerve = res_container->GetNerveResource();

		const auto& implant_datas = res_implant.data();
		const auto& nerve_datas = res_nerve.GetNerveDataInVol();
		const float implant_margin = GlobalPreferences::GetInstance()->preferences_.objects.implant.collision_margin;

		std::vector<GLObject*> objs;
		std::vector<int> look_up;
		std::vector<glm::mat4> obj_model_mats;
		std::vector<int> selected_indices;

		int implant_selected_idx = res_implant.add_implant_id();
		if (implant_selected_idx < 0)
		{
			implant_selected_idx = res_implant.selected_implant_id();
		}

		for (const auto& elem : implant_datas) 
		{
			if (!implant_objs_[elem.first])
			{
				CreateImplantObj(elem.first);
			}

			ImplantObjGL* obj = implant_objs_[elem.first].get();

			const glm::mat4& translate = obj->GetTranslate(UIGLObjects::TYPE_VOLUME);
			const glm::mat4& rotate = obj->GetRotate(UIGLObjects::TYPE_VOLUME);
			glm::mat4 scale = glm::scale(glm::vec3(1.5f));


			float diameter = elem.second->platform_diameter() / 2.0f;
			float length = elem.second->total_length() / 2.0f;

			glm::mat4 margin_scale = glm::scale(glm::vec3(
				((diameter + implant_margin) / diameter),
				((diameter + implant_margin) / diameter),
				((length + implant_margin) / length))
			);

			objs.push_back(dynamic_cast<GLObject*>(obj));
			look_up.push_back(obj->id());
#if 0
			glm::mat4 norm_scale = glm::scale(2.0f / (elem.second->bounding_box_max() - elem.second->bounding_box_min()));
			obj_model_mats.push_back(translate*rotate*glm::inverse(norm_scale)*margin_scale*norm_scale);
#else
			obj_model_mats.push_back(translate * rotate * margin_scale);
#endif

			if (implant_selected_idx == obj->id())
			{
				selected_indices.push_back(look_up.size() - 1);
			}
			else if (elem.second->is_collide())
			{
				selected_indices.push_back(look_up.size() - 1);
			}
		}

		for (const auto& elem : nerve_datas) 
		{
			if (!nerve_objs_[elem.first])
			{
				CreateNerveObj(elem.first);
			}

			GLObject* obj = nerve_objs_[elem.first].get();

			objs.push_back(obj);
			look_up.push_back(-1);
			obj_model_mats.push_back(glm::mat4(1.0f));
		}

		std::vector<int> collided_Indices;
		Collide collide;
		if (implant_selected_idx >= 0 && is_move_implant)
		{
			collide.run(prog_render_, projection_view, objs, obj_model_mats, collided_Indices, selected_indices);
		}
		else
		{
			collide.run(prog_render_, projection_view, objs, obj_model_mats, collided_Indices);
		}

		for (const auto& elem : collided_Indices) 
		{
			int id = look_up[elem];
			if (id > 0)
			{
				collided_ids->push_back(look_up[elem]);
			}
		}
		ReleaseFrameBufferObject();

#if DEVELOP_MODE
		common::Logger::instance()->PrintDebugMode("ImplantCollisionRenderer::CheckCollide");
#endif 

	}
	DoneCurrent();
}

void ImplantCollisionRenderer::ConnectObjGL() {
	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigUpdateNerve(this, SLOT(slotUpdateNerve()));
	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigUpdateImplant(this, SLOT(slotUpdateImplant()));

	is_connted_obj_gl_ = true;
}

void ImplantCollisionRenderer::CreateImplantObj(int implant_id) {
	implant_objs_[implant_id].reset(new ImplantObjGL(implant_id));
}

void ImplantCollisionRenderer::CreateNerveObj(int nerve_id) {
	nerve_objs_[nerve_id].reset(new GLObject);
	connect(nerve_objs_[nerve_id].get(), SIGNAL(sigInitialize()), this, SLOT(slotNerveInitializeVAOVBO()));
}

void ImplantCollisionRenderer::slotUpdateImplant() {
	is_update_implant_ = true;
}

void ImplantCollisionRenderer::slotUpdateNerve() {
	is_update_nerve_ = true;
}

void ImplantCollisionRenderer::slotNerveInitializeVAOVBO() {
	int nerve_id;
	auto logger = common::Logger::instance();

	for (const auto& elem : nerve_objs_) {
		if (QObject::sender() == (QObject*)elem.second.get())
			nerve_id = elem.first;
	}

	const auto& res_container = ResourceContainer::GetInstance();
	const NerveResource& res_nerve = res_container->GetNerveResource();

	const auto& nerve_datas = res_nerve.GetNerveDataInVol();
	const auto& iter = nerve_datas.find(nerve_id);
	if (iter == nerve_datas.end()) {
		logger->Print(common::LogType::ERR,
					  "ImplantCollisionRenderer::slotNerveInitializeVAOVBO: invalid nerve id: " +
					  QString(nerve_id).toStdString());
		return;
	}

	const NerveData& nerve_data = *(iter->second.get());
	
	const std::vector<glm::vec3>& verts = nerve_data.mesh_vertices();
	const std::vector<glm::vec3>& norms = nerve_data.mesh_normals();
	const std::vector<uint>& indices = nerve_data.mesh_indices();

	if (!nerve_objs_[nerve_id]) {
		logger->Print(common::LogType::ERR,
					  "ImplantCollisionRenderer::slotNerveInitializeVAOVBO: invalid nerve id: " +
					  QString(nerve_id).toStdString());
		return;
	}

	nerve_objs_[nerve_id]->InitVAOVBOmesh(verts, norms, indices);

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("ImplantCollisionRenderer::slotNerveInitializeVAOVBO");
#endif 

}

void ImplantCollisionRenderer::ClearNerveVAOVBO() {
	for (const auto& elem : nerve_objs_)
		elem.second->ClearVAOVBO();
	nerve_objs_.clear();
}

void ImplantCollisionRenderer::ClearImplantVAOVBO() {
	for (const auto& elem : implant_objs_)
		elem.second->ClearVAOVBO();
	implant_objs_.clear();
}
