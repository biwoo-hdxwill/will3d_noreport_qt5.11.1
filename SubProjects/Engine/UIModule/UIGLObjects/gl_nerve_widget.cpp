#include "gl_nerve_widget.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/Common/event_handler.h"
#include "../../Common/Common/event_handle_common.h"

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/nerve_resource.h"

#include "gl_object.h"

using namespace UIGLObjects;

GLNerveWidget::GLNerveWidget(ObjCoordSysType type)
	: type_(type) {
	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigUpdateNerve(this, SLOT(slotUpdateNerve()));
}

GLNerveWidget::~GLNerveWidget() {
}

void GLNerveWidget::Render(uint program) {
	if (is_update) {
		ClearVAOVBO();
		is_update = false;
	}
	this->SetUniformMatrix(program);
	WGLSLprogram::setUniform(program, "VolTexTransformMat", glm::inverse(world_));

	this->SetLightPosition();
	this->SetUniformLight(program);

	const std::map<int, std::unique_ptr<NerveData>>& nerve_datas = GetNerveDatasDependingType();

	for (const auto& elem : nerve_datas) {
		if (!elem.second->is_visible() || !elem.second->IsInitialize())
			continue;

		if (!objs_[elem.first])
			CreateObj(elem.first);

		QColor color = GetNerveDataFromResource(elem.first).color();

		this->SetColor(color);
		this->SetUniformColor(program);
		objs_[elem.first]->Render();
	}
}
void GLNerveWidget::RenderThicknessSlice(uint program,
										 GLObject* slice_obj,
										 const glm::mat4& slice_mvp_front, const glm::mat4& slice_mvp_back,
										 const glm::vec4 & slice_equ_front, const glm::vec4 & slice_equ_back) {
	WGLSLprogram::setUniform(program, "use_clipping", false);

	this->RenderSlice(program, slice_obj, slice_mvp_front);
	this->RenderSlice(program, slice_obj, slice_mvp_back);


	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 0, 0);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glDisable(GL_DEPTH_TEST);

	WGLSLprogram::setUniform(program, "MVP", slice_mvp_back);
	slice_obj->Render();
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

	WGLSLprogram::setUniform(program, "back_plane", slice_equ_back);
	WGLSLprogram::setUniform(program, "front_plane", slice_equ_front);
	WGLSLprogram::setUniform(program, "use_clipping", true);
	this->Render(program);
	glDisable(GL_STENCIL_TEST);
}
void GLNerveWidget::RenderSlice(uint program, GLObject* slice_obj, const glm::mat4& slice_mvp) {

	if (is_update) {
		ClearVAOVBO();
		is_update = false;
	}

	WGLSLprogram::setUniform(program, "use_clipping", false);

	const std::map<int, std::unique_ptr<NerveData>>& nerve_datas = GetNerveDatasDependingType();

	for (const auto& elem : nerve_datas) {
		if (!elem.second->is_visible() || !elem.second->IsInitialize())
			continue;

		if (!objs_[elem.first])
			CreateObj(elem.first);

		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDepthFunc(GL_LESS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		WGLSLprogram::setUniform(program, "MVP", slice_mvp);
		slice_obj->Render();

		this->SetUniformMatrix(program);
		WGLSLprogram::setUniform(program, "VolTexTransformMat", glm::inverse(world_));

		this->SetLightPosition();
		this->SetUniformLight(program);
		QColor color = GetNerveDataFromResource(elem.first).color();

		this->SetColor(color);
		this->SetUniformColor(program);

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, 0);
		glDepthMask(GL_FALSE);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LESS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		objs_[elem.first]->Render(GL_BACK);

		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		objs_[elem.first]->Render(GL_FRONT);

		glDepthFunc(GL_GREATER);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		objs_[elem.first]->Render(GL_FRONT);

		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		objs_[elem.first]->Render(GL_BACK);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilFunc(GL_EQUAL, 2, 2);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		objs_[elem.first]->Render();

	}


	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void GLNerveWidget::ClearVAOVBO() {
	for (const auto& elem : objs_)
		elem.second->ClearVAOVBO();

	objs_.clear();
}

const NerveData& GLNerveWidget::GetNerveDataFromResource(int nerve_id) const {
	const auto& nerve_datas = GetNerveDatasDependingType();
	auto iter = nerve_datas.find(nerve_id);

	if (iter == nerve_datas.end()) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "GLNerveWidget::set_points: invalid nerve id.");
		assert(false);
	}

	return *(iter->second.get());
}

const std::map<int, std::unique_ptr<NerveData>>& GLNerveWidget::GetNerveDatasDependingType() const {
	auto res_container = ResourceContainer::GetInstance();

	const NerveResource& res_nerve = res_container->GetNerveResource();

	if (type_ == ObjCoordSysType::TYPE_VOLUME) {
		return res_nerve.GetNerveDataInVol();
	} else if (type_ == ObjCoordSysType::TYPE_PANORAMA) {
		return res_nerve.GetNerveDataInPano();
	} else assert(false);

	return *(new std::map<int, std::unique_ptr<NerveData>>());
}

int GLNerveWidget::GetEventSenderIDfromObjs() const {
	for (const auto& elem : objs_) {
		if (QObject::sender() == (QObject*)elem.second.get())
			return elem.first;
	}

	return -1;
}

void GLNerveWidget::CreateObj(int nerve_id) {
	objs_[nerve_id].reset(new GLObject());
	connect(objs_[nerve_id].get(), SIGNAL(sigInitialize()), this, SLOT(slotNerveInitializeVAOVBO()));
}

void GLNerveWidget::SetColor(const QColor& color) {
	glm::vec3 colr = glm::vec3((float)color.red() / 255.0f,
		(float)color.green() / 255.0f,
							   (float)color.blue() / 255.0f);
	Material material;
	material.Ks = glm::vec3(1.0f);
	material.Ka = colr * 0.2f;
	material.Kd = colr;
	material.Shininess = 10.0f;

	set_material(material);

}

void GLNerveWidget::slotUpdateNerve() {
	is_update = true;
}

void GLNerveWidget::slotNerveInitializeVAOVBO() {
	int nerve_id = GetEventSenderIDfromObjs();

	const NerveData& nerve_data = GetNerveDataFromResource(nerve_id);

	const std::vector<glm::vec3>& verts = nerve_data.mesh_vertices();
	const std::vector<glm::vec3>& norms = nerve_data.mesh_normals();
	const std::vector<uint>& indices = nerve_data.mesh_indices();

	if (!objs_[nerve_id]) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR,
						 "GLNerveWidget::slotNerveInitializeVAOVBO: invalid nerve id: " +
						 QString(nerve_id).toStdString());
		return;
	}

	objs_[nerve_id]->InitVAOVBOmesh(verts, norms, indices);

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("GLNerveWidget::slotNerveInitializeVAOVBO");
#endif 
}
