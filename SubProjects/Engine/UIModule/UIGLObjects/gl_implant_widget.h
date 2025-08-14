#pragma once

/**=================================================================================================

Project:		UIGLObjects
File:			gl_implant_widget.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-20
Last modify: 	2018-04-20

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <map>
#include <memory>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include "gl_widget.h"

#include <QColor>

class ImplantData;
class ImplantObjGL;
class ViewPlaneObjGL;
class CW3SurfaceLineItem;

/** A implnat object gl widget. GLObject의 집합. */
class UIGLOBJECTS_EXPORT GLImplantWidget : public GLWidget {
	Q_OBJECT
public:

	/**=================================================================================================
	Constructor.

	Parameters:
	type - volume좌표계인지, panorama좌표계인지 결정한다.
	*===============================================================================================**/
	explicit GLImplantWidget(UIGLObjects::ObjCoordSysType type);
	~GLImplantWidget();

	GLImplantWidget(const GLImplantWidget&) = delete;
	GLImplantWidget& operator=(const GLImplantWidget&) = delete;

public:
	/**=================================================================================================
	Renders the slice. slice plane 기준으로 implant bounding box의 두배 크기만큼만 랜더링한다.
	
	Parameters:
	program - 	  	The program.
	slice_plane - 	The slice plane.
	 *===============================================================================================**/
	void RenderSlice(uint program, const glm::vec4& slice_plane);
	void RenderSliceWire(uint program, const glm::vec4& slice_plane, const float line_width = 1.0f);

	//void RenderSlice(uint program, GLObject * slice_obj, const glm::mat4 & slice_mvp);

	/**=================================================================================================
	Picks the implant. slice plane에서 가장 가까운 implant id를 리턴한다.

	Parameters:
	program - 	  	The program.
	slice_plane - 	The slice plane.
	pt_read_pixel - glReadPixel함수에 들어가는 픽셀 위치.
	
	Returns:	implant id
	 *===============================================================================================**/
	int PickSlice(uint program, const glm::vec4 & slice_plane, const glm::vec2& pt_read_pixel);

	void RenderSingleImplant(uint program, int implant_id);

	void Render(uint program);
	void RenderForPick(uint program);
	
	void ClearVAOVBO();

	inline void set_world(const glm::mat4& mat) { world_ = mat; }

	inline const std::map<int, bool>& visibility() { return visibility_; }

	void ApplyPreferences();

private:
	void CreateImplantObj(int implant_id);
	void CreateImplantMajorAxes(int implant_id, const glm::vec3 & major_point);
	bool SetUniformSliceImplantObj(uint program, const glm::vec4 & slice_plane, const ImplantObjGL& implant_gl);
	void SetUniformsImplantObj(uint program, const ImplantObjGL& implant_gl);

	void SetUniformsImplantMajorAxis(CW3SurfaceLineItem * implant_major_axis);
	void RenderMajorAxis(uint program, uint implant_id);

	void SetColor(const QColor& color);
private slots:

	/** Slot update implant. ResourceContainer의 implant mesh데이터가 업데이트되면 호출된다.  */
	void slotUpdateImplant();

private:
	std::map<int, std::unique_ptr<ImplantObjGL>> objs_;
	std::map<int, std::unique_ptr<CW3SurfaceLineItem>> major_axes_;
	std::map<int, bool> visibility_;

	UIGLObjects::ObjCoordSysType type_;
	bool is_update = false;
	glm::mat4 world_;

	QColor default_color_volume_;
	QColor default_color_wire_;
	QColor selected_color_volume_;
	QColor selected_color_wire_;
	QColor collided_color_volume_;
	QColor collided_color_wire_;
};
