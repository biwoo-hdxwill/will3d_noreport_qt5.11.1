#pragma once

/**=================================================================================================

Project:		UIGLObjects
File:			gl_nerve_widget.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-12-05
Last modify: 	2017-12-05

	Copyright (c) 2017 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <map>
#include <memory>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include "gl_widget.h"

class NerveData;
class GLObject;

/** A nerve object gl widget. GLObject의 집합. */
class UIGLOBJECTS_EXPORT GLNerveWidget : public GLWidget {
	Q_OBJECT
public:

	/**=================================================================================================
	Constructor.

	Parameters:
	type - volume좌표계인지, panorama좌표계인지 결정한다.
	*===============================================================================================**/
	explicit GLNerveWidget(UIGLObjects::ObjCoordSysType type);
	~GLNerveWidget();

	GLNerveWidget(const GLNerveWidget&) = delete;
	GLNerveWidget& operator=(const GLNerveWidget&) = delete;

public:
	void Render(uint program);
	void RenderThicknessSlice(uint program,
							  GLObject* slice_obj, 
							  const glm::mat4& slice_mvp_front, const glm::mat4& slice_mvp_back,
							  const glm::vec4 & slice_equ_front, const glm::vec4 & slice_equ_back);
	void RenderSlice(uint program, GLObject * slice_obj, const glm::mat4 & slice_mvp);
	void ClearVAOVBO();

	inline void set_world(const glm::mat4& mat) { world_ = mat; }
private:
	const NerveData& GetNerveDataFromResource(int nerve_id) const;
	const std::map<int, std::unique_ptr<NerveData>>& GetNerveDatasDependingType() const;

	int GetEventSenderIDfromObjs() const;

	void CreateObj(int nerve_id);
	void SetColor(const QColor & color);
private slots:

	/** Slot nerve initialize vao&vbo. NerveResource에서 nerve mesh 데이터들을 GLObject에 셋팅한다.
	GLObject가 render할 때 VAO가 null이면 이 함수가 호출되어 vao를 초기화한다.
	*/
	void slotNerveInitializeVAOVBO();


	/** Slot update nerve. ResourceContainer의 nerve mesh데이터가 업데이트되면 호출된다.  */
	void slotUpdateNerve();


private:
	UIGLObjects::ObjCoordSysType type_;

	std::map<int, std::unique_ptr<GLObject>> objs_;

	bool is_update = false;
	glm::mat4 world_;
};
