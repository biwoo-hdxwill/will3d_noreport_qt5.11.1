#include "W3ViewPlane.h"
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
#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Math.h"
#include "../../Common/Common/W3Memory.h"

CW3ViewPlane::~CW3ViewPlane(void) {
  SAFE_DELETE_OBJECT(m_pImg);
  SAFE_DELETE_OBJECT(m_pMask);
}

void CW3ViewPlane::setZeros(void) {
  if (m_pImg) m_pImg->setZeros();
}

void CW3ViewPlane::setVectors(const glm::vec3 &pos, const glm::vec3 &up,
                              const glm::vec3 &back, const glm::vec3 &vol_range,
                              bool isInit) {
  plane_center_ = pos;

  m_up = glm::normalize(up);  // 모니터 뚫고 지나가는 방향
  m_back = glm::normalize(back);
  // set right vector.
  m_right = glm::cross(m_up, m_back);
  m_right = glm::normalize(m_right);

  if (isInit) {
    vol_range_ = vol_range;
    vol_center_ = pos;
    dist_from_vol_center_ = 0.0f;
  }
}

void CW3ViewPlane::setDimensions(int width, int height) {
  m_pImg->setWidth(width);
  m_pImg->setHeight(height);
}

void CW3ViewPlane::createImage2D(int width, int height) {
  if (width > 0 && height > 0) {
    SAFE_DELETE_OBJECT(m_pImg);
    m_pImg = new CW3Image2D(width, height);
    SAFE_DELETE_OBJECT(m_pMask);
    W3::p_allocate_1D(&m_pMask, width * height);
  }
}

void CW3ViewPlane::resize(int width, int height) {
  if (!m_pImg) return;

  m_pImg->resize(width, height);

  SAFE_DELETE_ARRAY(m_pMask);
  W3::p_allocate_1D(&m_pMask, width * height);
}

float CW3ViewPlane::translate(const float fDist, glm::vec3 &vCrossingPt, const bool limit_move_range) {
  glm::vec3 vTrans = fDist * m_up;
  glm::vec3 tempCrossingPt = vCrossingPt + vTrans;
  glm::vec3 dest_cross_pos = limit_move_range ? FitVolRange(vCrossingPt, tempCrossingPt) : tempCrossingPt;

  float sign = fDist > 0 ? 1 : (fDist == 0 ? 0 : -1);
  float fittedDist = sign * glm::length(dest_cross_pos - vCrossingPt);
  dist_from_vol_center_ += fittedDist;
  vCrossingPt = dest_cross_pos;

  plane_center_ = vol_center_ + dist_from_vol_center_ * m_up;

  return fittedDist;
}

glm::vec3 CW3ViewPlane::FitVolRange(const glm::vec3& cross_point, const glm::vec3& destination) 
{
	return W3::FitVolRange(vol_range_, cross_point, destination);  
}

void CW3ViewPlane::translateSecond(const float fittedDist) {
  dist_from_vol_center_ += fittedDist;
  plane_center_ = vol_center_ + dist_from_vol_center_ * m_up;
}

void CW3ViewPlane::translateSecond(glm::vec3 &planeCenter) {
  plane_center_ = planeCenter;
}

void CW3ViewPlane::rotate3D(glm::mat4 &T, const glm::vec3 &rotCenter,
                            const glm::mat4 &Normals) {
  glm::mat4 preNormals(1.0f);
  preNormals[0] = glm::vec4(m_right, 0.0f);
  preNormals[1] = glm::vec4(m_back, 0.0f);
  preNormals[2] = glm::vec4(m_up, 0.0f);

  T = Normals * glm::inverse(preNormals);

  m_right = glm::vec3(Normals[0]);
  m_back = glm::vec3(Normals[1]);
  m_up = glm::vec3(Normals[2]);

  dist_from_vol_center_ = glm::dot(rotCenter - vol_center_, m_up);

  plane_center_ = vol_center_ + dist_from_vol_center_ * m_up;
}

void CW3ViewPlane::rotate3Dpassive(const glm::vec3 &rotCenter,
                                   const glm::mat4 &rotMat) {
  glm::vec4 vUp(m_up, 0.0f);
  glm::vec4 vBack(m_back, 0.0f);
  glm::vec4 vRight(m_right, 0.0f);

  glm::vec4 up4 = glm::normalize(rotMat * vUp);
  glm::vec4 back4 = glm::normalize(rotMat * vBack);
  glm::vec4 right4 = glm::normalize(rotMat * vRight);

  m_up = up4.xyz;
  m_back = back4.xyz;
  m_right = right4.xyz;

  dist_from_vol_center_ = glm::dot(rotCenter - vol_center_, m_up);

  plane_center_ = vol_center_ + dist_from_vol_center_ * m_up;
}

float CW3ViewPlane::rotate(const glm::vec3 &rotCenter, const glm::vec3 &vAxis,
                           const float fAngle) {
  glm::vec4 vUp(m_up, 0.0f);
  glm::vec4 vBack(m_back, 0.0f);

  glm::mat4 rotMat = glm::rotate(fAngle, vAxis);
  rotate_matrix_ = rotMat * rotate_matrix_;

  glm::vec4 up4 = rotMat * vUp;
  glm::vec4 back4 = rotMat * vBack;

  m_up = up4.xyz;
  m_back = back4.xyz;

  setRightVec();

  dist_from_vol_center_ = glm::dot(rotCenter - vol_center_, m_up);
  plane_center_ = vol_center_ + dist_from_vol_center_ * m_up;

  return dist_from_vol_center_;
}

void CW3ViewPlane::rotateSecond(const float dist, const glm::vec3 &vAxis,
                                const float fAngle) {
  glm::vec4 vUp(m_up, 0.0f);
  glm::vec4 vBack(m_back, 0.0f);

  glm::mat4 rotMat = glm::rotate(fAngle, vAxis);

  glm::vec4 up4 = rotMat * vUp;
  glm::vec4 back4 = rotMat * vBack;

  m_up = up4.xyz;
  m_back = back4.xyz;

  setRightVec();

  plane_center_ = vol_center_ + dist * m_up;
}

void CW3ViewPlane::rotateSecond(const glm::vec3 &planeCenter,
                                const glm::vec3 &vAxis, const float fAngle) {
  glm::vec4 vUp(m_up, 0.0f);
  glm::vec4 vBack(m_back, 0.0f);

  glm::mat4 rotMat = glm::rotate(fAngle, vAxis);

  glm::vec4 up4 = rotMat * vUp;
  glm::vec4 back4 = rotMat * vBack;

  m_up = up4.xyz;
  m_back = back4.xyz;

  setRightVec();

  plane_center_ = planeCenter;
}
