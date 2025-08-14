#pragma once
/*=========================================================================

File:		class CW3Point3D
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include "common_global.h"
class COMMON_EXPORT CW3Point3D
{
public:
	CW3Point3D(void) : X(0), Y(0), Z(0) {};
	~CW3Point3D(void) {};
	/****************************
		Constructors.
		*************************/
	CW3Point3D(float x, float y, float z) : X(x), Y(y), Z(z) {}
	inline CW3Point3D(const CW3Point3D& pt) : X(pt.X), Y(pt.Y), Z(pt.Z) {} // copy constructor.
	/****************************
		Operator Overloadings.
		*************************/
	inline CW3Point3D& operator=(const CW3Point3D& pt){ // copy assignment.
		X = pt.X;
		Y = pt.Y;
		Z = pt.Z;
		return *this;}
	inline CW3Point3D operator-(const CW3Point3D& pt){
		return CW3Point3D(this->x() - pt.x(), this->y() - pt.y(), this->z() - pt.z());}
	inline CW3Point3D operator+(const CW3Point3D& pt){
		return CW3Point3D(this->x() + pt.x(), this->y() + pt.y(), this->z() + pt.z());}

	inline bool operator!=(const CW3Point3D& pt){
		return ((this->X == pt.X ) && (this->Y == pt.Y) && (this->Z == pt.Z)) ? false : true;}
	inline bool operator==(const CW3Point3D& pt){
		return ((this->X == pt.X ) && (this->Y == pt.Y) && (this->Z == pt.Z)) ? true : false;}
	// support minus operator.
	inline CW3Point3D operator-(void){
		return CW3Point3D(-this->X, -this->Y, -this->Z);}

public:
	// public functions.
	inline float x(void) const		{ return X; }
	inline float y(void) const		{ return Y; }
	inline float z(void) const		{ return Z; }
	inline void setX(const float x)	{ this->X = x; }
	inline void setY(const float y)	{ this->Y = y; }
	inline void setZ(const float z)	{ this->Z = z; }
	inline void set(const float x, const float y, const float z)	{ this->X = x; this->Y = y;this->Z = z; }

	/*
		Move position.
	*/
	inline void moveBy(const float dx, const float dy, const float dz) {
		this->X += dx;
		this->Y += dy;
		this->Z += dz;}

private:
	// private fields.
	float X;
	float Y;
	float Z;
};

