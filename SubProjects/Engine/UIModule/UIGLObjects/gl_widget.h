#pragma once
/**=================================================================================================

Project:		UIGLObjects
File:			gl_widget.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-12-05
Last modify: 	2017-12-05

	Copyright (c) 2017 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "uiglobjects_global.h"
#include "uiglobjects_defines.h"
#include <QObject>

class UIGLOBJECTS_EXPORT GLWidget : public QObject {
	Q_OBJECT
public:
	GLWidget();
	virtual ~GLWidget();

	GLWidget(const GLWidget&) = delete;
	const GLWidget& operator=(const GLWidget&) = delete;

public:
	void EditTransformMat(const glm::mat4& mat4, UIGLObjects::TransformType type);
	void SetTransformMat(const glm::mat4& mat4, UIGLObjects::TransformType type);
	void SetAlpha(double alpha);
	inline void set_projection(const glm::mat4& projection) { projection_ = projection; }
	inline void set_view(const glm::mat4& view) { view_ = view; }
	inline void set_material(const UIGLObjects::Material& material_color) { material_ = material_color; }
	inline void set_is_visible(bool is_visible) { is_visible_ = is_visible; }
	inline void set_light(const UIGLObjects::Light& light_info) { light_ = light_info; }

	const glm::mat4& GetTransformMat(UIGLObjects::TransformType type);
	glm::mat4 GetVolTexTransformMat();

	inline bool is_visible() const { return is_visible_; }
	inline bool is_transparency() const { return is_transparency_; }
	inline double alpha() const { return alpha_; }

	inline const glm::mat4& model() const { return model_; }
	inline const glm::mat4& view() const { return view_; }
	inline const glm::mat4& projection() const { return projection_; }
	inline const glm::mat4& mvp() const { return mvp_; }
	inline const glm::mat4& mv() const { return mv_; }
	inline const UIGLObjects::Material& material() const { return material_; }
protected:
	void SetUniformColor(uint program);
	void SetUniformMatrix(uint program);
	void SetUniformLight(uint program);

	void ReadPickInfo(int x, int y, unsigned char* pick_id, glm::vec3* pick_gl_coord);
	virtual void SetLightPosition();

private:
	UIGLObjects::TransformMat transform_;
	glm::vec3 centroid_;

	UIGLObjects::Material material_;
	bool is_visible_ = true;
	bool is_transparency_ = false;
	double alpha_ = 1.0f;

	UIGLObjects::Light light_;

	glm::mat4 model_;
	glm::mat4 view_;
	glm::mat4 projection_;
	glm::mat4 mvp_;
	glm::mat4 mv_;
};
