#include "gl_implant_widget.h"

#include <QDebug>
#include <QSettings>

#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/event_handler.h"
#include "../../Common/Common/event_handle_common.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/implant_resource.h"

#include "implant_obj_gl.h"
#include "view_plane_obj_gl.h"
#include "W3SurfaceLineItem.h"

using namespace UIGLObjects;

GLImplantWidget::GLImplantWidget(ObjCoordSysType type) : type_(type) {
	ApplyPreferences();

	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigUpdateImplant(this, SLOT(slotUpdateImplant()));
}

GLImplantWidget::~GLImplantWidget() {
	ClearVAOVBO();
}

void GLImplantWidget::ApplyPreferences()
{
	default_color_volume_ = GlobalPreferences::GetInstance()->preferences_.objects.implant.default_color_volume;
	default_color_wire_ = GlobalPreferences::GetInstance()->preferences_.objects.implant.default_color_wire;
	selected_color_volume_ = GlobalPreferences::GetInstance()->preferences_.objects.implant.selected_color_volume;
	selected_color_wire_ = GlobalPreferences::GetInstance()->preferences_.objects.implant.selected_color_wire;
	collided_color_volume_ = GlobalPreferences::GetInstance()->preferences_.objects.implant.collided_color_volume;
	collided_color_wire_ = GlobalPreferences::GetInstance()->preferences_.objects.implant.collided_color_wire;

	double alpha = GlobalPreferences::GetInstance()->preferences_.objects.implant.alpha;
	SetAlpha(alpha);
}

void GLImplantWidget::RenderSlice(uint program, const glm::vec4& slice_plane) {
	if (is_update) {
		ClearVAOVBO();
		is_update = false;
	}
	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	if (!res_implant.is_visible_all())
		return;

	this->SetTransformMat(mat4(1.0f), TransformType::SCALE);
	this->SetLightPosition();
	this->SetUniformLight(program);

	WGLSLprogram::setUniform(program, "clip_plane", slice_plane);

	const auto& implant_datas = res_implant.data();
	for (const auto& elem : implant_datas) {
#if 1
		if (!elem.second->is_visible())
		{
			continue;
		}
#endif

		if (!objs_[elem.first]) {
			CreateImplantObj(elem.first);
		}
		if (!major_axes_[elem.first]) {
			CreateImplantMajorAxes(elem.first, elem.second->major_axis());
		}
		WGLSLprogram::setUniform(program, "use_clipping", true);
		ImplantObjGL* obj = objs_[elem.first].get();

		SetUniformsImplantObj(program, *obj);
		bool is_res = SetUniformSliceImplantObj(program, slice_plane, *obj);

		if (!is_res) {
			continue;
		}

		obj->Render();

		WGLSLprogram::setUniform(program, "use_clipping", false);
		RenderMajorAxis(program, elem.first);
	}
}

void GLImplantWidget::RenderSliceWire(uint program, const glm::vec4& slice_plane, const float line_width)
{
	if (is_update)
	{
		ClearVAOVBO();
		is_update = false;
	}

	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	if (!res_implant.is_visible_all())
		return;

	SetTransformMat(mat4(1.0f), TransformType::SCALE);

	const auto& implant_datas = res_implant.data();
	for (const auto& elem : implant_datas)
	{
		if (!elem.second->is_visible())
			continue;

		if (!objs_[elem.first])
		{
			CreateImplantObj(elem.first);
		}

		ImplantObjGL* obj = objs_[elem.first].get();

		const glm::mat4& translate = obj->GetTranslate(type_);
		const glm::mat4& rotate = obj->GetRotate(type_);
		WGLSLprogram::setUniform(program, "implant_id", (obj->id() + 1) / 255.0f);
		SetTransformMat(translate, TransformType::TRANSLATE);
		SetTransformMat(rotate, TransformType::ROTATE);
		SetUniformMatrix(program);

		QColor color = default_color_wire_;
		if (obj->IsCollided())
		{
			color = collided_color_wire_;
		}
		else if (obj->IsSelected())
		{
			color = selected_color_wire_;
		}

		WGLSLprogram::setUniform(program, "invert_x", false);
		WGLSLprogram::setUniform(program, "isYinvert", false);
		WGLSLprogram::setUniform(program, "isWire", true);
		WGLSLprogram::setUniform(program, "meshColor", glm::vec3(color.redF(), color.greenF(), color.blueF()));

		bool is_res = SetUniformSliceImplantObj(program, slice_plane, *obj);

		if (!is_res)
		{
			continue;
		}

		obj->RenderWire(line_width);
	}
}

int GLImplantWidget::PickSlice(uint program, const glm::vec4& slice_plane, const glm::vec2& pt_read_pixel) {
	if (is_update) {
		ClearVAOVBO();
		is_update = false;
	}

	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	if (!res_implant.is_visible_all())
		return -1;

	this->SetLightPosition();
	this->SetUniformLight(program);

	WGLSLprogram::setUniform(program, "use_clipping", false);

	auto func_render_implant = [&](const std::map<int, ImplantData*>& implant_datas) {
		for (const auto& elem : implant_datas) {
			if (!elem.second->is_visible())
				continue;

			if (!objs_[elem.first])
				CreateImplantObj(elem.first);


			ImplantObjGL* obj = objs_[elem.first].get();

			SetUniformsImplantObj(program, *obj);
			bool is_res = SetUniformSliceImplantObj(program, slice_plane, *obj);

			if (!is_res)
			{
				continue;
			}

			glEnable(GL_DEPTH_TEST);
			obj->Render();
			glDisable(GL_DEPTH_TEST);
		}
	};

	const auto& implant_datas = res_implant.data();

	bool is_pick_end = false;
	int pick_id = -1;
	glm::vec3 up_vector = glm::vec3(slice_plane.x, slice_plane.y, slice_plane.z);
	float slice_to_implant_min_dist = std::numeric_limits<float>::max();

	std::vector<int> candidate_picking_ids;
	candidate_picking_ids.reserve(implant_datas.size());
	for (const auto& elem : implant_datas) {
		candidate_picking_ids.push_back(elem.first);
	}

	std::map<int, ImplantData*> candidate_implant_datas;
	for (const auto& elem : candidate_picking_ids) {
		candidate_implant_datas[elem] = implant_datas.at(elem).get();
	}

	while (!is_pick_end) {
		CW3GLFunctions::clearView(true);
		func_render_implant(candidate_implant_datas);

		glm::vec4 res = CW3GLFunctions::readPickColor(pt_read_pixel, GL_RGBA, GL_FLOAT);

		int id = (int)(res.w*255.0f);

		if (id > 0.0f &&
			candidate_implant_datas.find(id) != candidate_implant_datas.end()) {

			candidate_implant_datas.erase(id);

			ImplantObjGL* implant = objs_[id].get();
			glm::vec3 bounding_max, bounding_min;
			implant->GetBoundingBoxRange(&bounding_max, &bounding_min);
			glm::vec3 implant_position = vec3(implant->GetTranslate(type_) *
											  implant->GetRotate(type_) *
											  vec4((bounding_min + bounding_max) * 0.5f, 1.0f));

			float slice_to_implant_dist = abs(slice_plane.w + glm::dot(implant_position, up_vector));
			if (slice_to_implant_min_dist > slice_to_implant_dist) {
				slice_to_implant_min_dist = slice_to_implant_dist;
				pick_id = id;
			}
		}
		else {
			is_pick_end = true;
		}
	}
	return pick_id;

}
void GLImplantWidget::RenderSingleImplant(uint program, int implant_id) {
	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	if (!res_implant.is_visible_all())
		return;

	const auto& implant_datas = res_implant.data();
	if (implant_datas.find(implant_id) == implant_datas.end())
		return;


	if (is_update) {
		ClearVAOVBO();
		is_update = false;
	}

	if (!objs_[implant_id])
		CreateImplantObj(implant_id);

	ImplantObjGL* obj = objs_[implant_id].get();

	this->SetUniformMatrix(program);
	this->SetUniformColor(program);

	obj->Render();
}
void GLImplantWidget::Render(uint program) {
	if (is_update) {
		ClearVAOVBO();
		is_update = false;
	}
	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	if (!res_implant.is_visible_all())
		return;

	const auto& implant_datas = res_implant.data();

	for (const auto& elem : implant_datas) {
		if (!elem.second->is_visible())
			continue;


		CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");
		if (!objs_[elem.first]) {
			CreateImplantObj(elem.first);
		}

		CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");

		if (!major_axes_[elem.first]) {
			CreateImplantMajorAxes(elem.first, elem.second->major_axis());
		}

		CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");
		ImplantObjGL* obj = objs_[elem.first].get();

		SetUniformsImplantObj(program, *obj);

		CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");
		obj->Render();


		CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");
		RenderMajorAxis(program, elem.first);

		CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");
	}
}

void GLImplantWidget::RenderForPick(uint program) {
	if (is_update) {
		ClearVAOVBO();
		is_update = false;
	}
	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	if (!res_implant.is_visible_all())
		return;

	const auto& implant_datas = res_implant.data();

	for (const auto& elem : implant_datas) {
		if (!elem.second->is_visible())
			continue;

		if (!objs_[elem.first])
			CreateImplantObj(elem.first);

		ImplantObjGL* obj = objs_[elem.first].get();

		const glm::mat4& translate = obj->GetTranslate(UIGLObjects::TYPE_VOLUME);
		const glm::mat4& rotate = obj->GetRotate(UIGLObjects::TYPE_VOLUME);

		SetUniformsImplantObj(program, *obj);
		obj->Render();
	}
}

void GLImplantWidget::ClearVAOVBO() {
	for (const auto& elem : objs_)
		elem.second->ClearVAOVBO();

	for (auto& elem : major_axes_)
		elem.second->clearVAOVBO();

	objs_.clear();
	major_axes_.clear();
}
void GLImplantWidget::CreateImplantObj(int implant_id) {
	objs_[implant_id].reset(new ImplantObjGL(implant_id));
	visibility_[implant_id] = false;
}
void GLImplantWidget::CreateImplantMajorAxes(int implant_id, const glm::vec3& major_point) {
	major_axes_[implant_id].reset(new CW3SurfaceLineItem(CW3SurfaceLineItem::CURVE, false));
	major_axes_[implant_id]->set_line_width(2.0f);
	std::vector<glm::vec3> major_points;
	major_points.push_back(major_point*-0.5f);
	major_points.push_back(major_point*0.5f);
	major_axes_[implant_id]->setPoints(major_points);
}
bool GLImplantWidget::SetUniformSliceImplantObj(uint program, const glm::vec4& slice_plane,
												const ImplantObjGL& implant_gl) {
	glm::mat4 model = this->model();
	WGLSLprogram::setUniform((GLuint)program, "ModelMatrix", model);

	glm::vec3 up_vector = glm::vec3(slice_plane.x, slice_plane.y, slice_plane.z);
	glm::vec3 bounding_max, bounding_min;
	if (implant_gl.IsValid()) {
		implant_gl.GetBoundingBoxRange(&bounding_max, &bounding_min);

		const float bounding_scale_offset = 1.0f;

		bounding_max *= bounding_scale_offset;
		bounding_min *= bounding_scale_offset;

		glm::vec3 p[8] = {
			bounding_min,
			glm::vec3(bounding_max.x, bounding_min.y, bounding_min.z),
			glm::vec3(bounding_max.x, bounding_min.y, bounding_max.z),
			glm::vec3(bounding_min.x, bounding_min.y, bounding_max.z),
			glm::vec3(bounding_min.x, bounding_max.y, bounding_min.z),
			glm::vec3(bounding_max.x, bounding_max.y, bounding_min.z),
			bounding_max,
			glm::vec3(bounding_min.x, bounding_max.y, bounding_max.z)
		};

		float dist_min = std::numeric_limits<float>::max();
		float dist_max = std::numeric_limits<float>::min();
		int min_idx, max_idx;

		for (int i = 0; i < 8; i++) {
			glm::vec3 mp = glm::vec3(implant_gl.GetRotate(type_)*vec4(p[i], 1.0));
			float dist = glm::dot(mp, up_vector);

			dist_min = std::min(dist_min, dist);
			dist_max = std::max(dist_max, dist);
		}
		glm::mat4 mat_translate = implant_gl.GetTranslate(type_);
		glm::vec3 translate = glm::vec3(glm::vec3(mat_translate[3][0],
										mat_translate[3][1],
										mat_translate[3][2]));

		float over_plane = (dist_min + dist_max)*0.5f + slice_plane.w + glm::dot(translate, up_vector);
		if (dist_max < over_plane || dist_min > over_plane)
		{
			visibility_[implant_gl.id()] = false;
			return false;
		}

		WGLSLprogram::setUniform(program, "back_plane", glm::vec4(up_vector, dist_max + slice_plane.w));
		WGLSLprogram::setUniform(program, "front_plane", glm::vec4(up_vector, dist_min + slice_plane.w));

		visibility_[implant_gl.id()] = true;
		return true;
	}

	visibility_[implant_gl.id()] = false;
	return false;
}

void GLImplantWidget::SetUniformsImplantObj(uint program, const ImplantObjGL& implant_gl) {
	const glm::mat4& translate = implant_gl.GetTranslate(type_);
	const glm::mat4& rotate = implant_gl.GetRotate(type_);
	WGLSLprogram::setUniform(program, "VolTexTransformMat", glm::inverse(world_) * translate * rotate);
	WGLSLprogram::setUniform(program, "index", implant_gl.id());
	this->SetTransformMat(translate, TransformType::TRANSLATE);
	this->SetTransformMat(rotate, TransformType::ROTATE);
	this->SetUniformMatrix(program);

	if (implant_gl.IsCollided())
	{
		SetColor(collided_color_volume_);
	}
	else if (implant_gl.IsSelected())
	{
		SetColor(selected_color_volume_);
	}
	else
	{
		SetColor(default_color_volume_);
	}

	this->SetUniformColor(program);

	this->SetLightPosition();
	this->SetUniformLight(program);
}
void GLImplantWidget::SetUniformsImplantMajorAxis(CW3SurfaceLineItem* implant_major_axis) {
	implant_major_axis->setTransformMat(this->GetTransformMat(TransformType::TRANSLATE), TransformType::TRANSLATE);
	implant_major_axis->setTransformMat(this->GetTransformMat(TransformType::ROTATE), TransformType::ROTATE);
	implant_major_axis->setTransformMat(this->GetTransformMat(TransformType::ARCBALL), TransformType::ARCBALL);
	implant_major_axis->setProjViewMat(this->projection(), this->view());

	glm::vec3 color = this->material().Kd;
	implant_major_axis->setLineColor(QColor(static_cast<int>(color.r * 255.0f),
									 static_cast<int>(color.g * 255.0f), 
									 static_cast<int>(color.b * 255.0f)));
}
void GLImplantWidget::RenderMajorAxis(uint program, uint implant_id) {
	CW3SurfaceLineItem* major_axis = major_axes_[implant_id].get();
	SetUniformsImplantMajorAxis(major_axis);

	major_axis->draw(program);
}
void GLImplantWidget::SetColor(const QColor& color) {
	glm::vec3 colr = glm::vec3((float)color.red() / 255.0f,
							   (float)color.green() / 255.0f,
							   (float)color.blue() / 255.0f);
	Material material;
#if 1
	material.Ks = glm::vec3(1.0f);
	material.Ka = colr * 0.2f;
	material.Kd = colr;
	material.Shininess = 50.0f;
#else
	material.Ks = glm::vec3(colr * 0.3f);
	material.Ka = material.Ks;
	material.Kd = material.Ks;
	material.Shininess = 3.0f;
#endif

	set_material(material);
}

void GLImplantWidget::slotUpdateImplant() {
	is_update = true;
}
