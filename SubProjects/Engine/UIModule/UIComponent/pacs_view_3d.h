#pragma once
/*=========================================================================
File:			pacs_view_3d.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-07-15
Last modify:	2021-08-25

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <GL/glm/detail/type_mat.hpp>
#include "view_3d.h"

class ViewControllerPACS3D;
class UICOMPONENT_EXPORT PACSView3D : public View3D 
{
	Q_OBJECT
public:
	explicit PACSView3D(QWidget* parent = 0);
	~PACSView3D();

	PACSView3D(const PACSView3D&) = delete;
	PACSView3D& operator=(const PACSView3D&) = delete;

	void SetVisibleNerve(bool is_visible);
	void SetVisibleImplant(bool is_visible);
	void ForceRotateMatrix(const glm::mat4& mat);

	void EmitCreateDCMFile(const int cnt);

	virtual BaseViewController3D* controller_3d() override;

	inline void set_angle(int angle) { angle_ = angle; }
	inline void set_num(int num) { num_ = num; }
	inline void set_anterior(bool anterior) { anterior_ = anterior; }
	inline void set_horizontal(bool horizontal) { horizontal_ = horizontal; }

signals:
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

private:
	void RenderVolume();

	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
	virtual void resizeEvent(QResizeEvent* event) override;

	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override {};
	
private:
	ViewControllerPACS3D* controller_ = nullptr;

	int angle_ = 1;
	int num_ = 0; //slice_num
	bool anterior_ = true;
	bool horizontal_ = true;
};
