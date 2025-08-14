#pragma once
/*=========================================================================

File:			class CAzViewPlane
Language:		C++11
Library:		Standard C++ Library
Author:			Hong Jung
First Date:		2015-11-24
Last Date:		2016-04-26
Version:		1.0

	Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#if defined(__APPLE__)
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#define GLM_SWIZZLE
#include <gl/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#endif

#include "W3Image2D.h"

#include "resource_global.h"

class RESOURCE_EXPORT CW3ViewPlane {
public:
	CW3ViewPlane() {}
	~CW3ViewPlane();

	void setVectors(const glm::vec3 &pos, const glm::vec3 &up, const glm::vec3 &back,
					const glm::vec3& vol_range, bool isInit);
	void setDimensions(int width, int height);

	void createImage2D(int width, int height);
	void resize(int width, int height);
	void setZeros();

	float translate(const float fDist, glm::vec3& vOrigin, const bool limit_move_range = true);
	void translateSecond(const float fittedDist);
	void translateSecond(glm::vec3 &planeCenter);
	void rotate3D(glm::mat4 &T, const glm::vec3& rotCenter, const glm::mat4 &Normals);
	void rotate3Dpassive(const glm::vec3& rotCenter, const glm::mat4 &rotate);
	float rotate(const glm::vec3& rotCenter, const glm::vec3 &vAxis, const float fAngle);
	void rotateSecond(const float dist, const glm::vec3 &vAxis, const float fAngle);
	void rotateSecond(const glm::vec3& planeCenter, const glm::vec3 &vAxis, const float fAngle);

	inline void	setRightVec(void) { m_right = glm::cross(m_up, m_back); }
	inline void	setBackVec(const glm::vec3 &back) { m_back = back; }
	inline void	set_up(const glm::vec3& up) { m_up = up; }
	inline void	setVolCenter(const glm::vec3 &v) { vol_center_ = v; }
	inline void setAvailableDepth(int depth) noexcept { available_depth_ = depth; };

	inline CW3Image2D* getImage2D(void) { return m_pImg; }
	inline unsigned char* getMask() { return m_pMask; }

	inline const glm::vec3& getPlaneCenterInVol(void) const noexcept { return plane_center_; }
	inline const glm::vec3& getUpVec(void) const noexcept { return m_up; }
	inline const glm::vec3& getRightVec(void) const noexcept { return m_right; }
	inline const glm::vec3& getBackVec(void) const noexcept { return m_back; }

	inline const int getWidth(void) const { return m_pImg->width(); }
	inline const int getHeight(void) const { return m_pImg->height(); }
	inline const glm::vec3 getVolCenter() const { return vol_center_; }
	inline int getAvailableDetph() const noexcept { return available_depth_; }
	inline float getDistFromVolCenter() const noexcept { return dist_from_vol_center_; }

	inline void	set_plane_center(const glm::vec3& value) { plane_center_ = value; }
	inline void set_dist_from_vol_center(const float value) { dist_from_vol_center_ = value; }

	inline void set_rotate_matrix(const glm::mat4& rot) { rotate_matrix_ = rot; }
	inline const glm::mat4& rotate_matrix() { return rotate_matrix_; }

//private:
	glm::vec3 FitVolRange(const glm::vec3& cross_point, const glm::vec3& destination);

private:
	CW3Image2D * m_pImg = nullptr;
	unsigned char* m_pMask = nullptr;

	glm::vec3 vol_range_ = glm::vec3(0.0f);
	glm::vec3 vol_center_ = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 plane_center_ = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 m_up = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_right = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_back = glm::vec3(0.0f, 0.0f, 0.0f);

	float dist_from_vol_center_ = 0.0f;
	int available_depth_;

	glm::mat4 rotate_matrix_;
};
