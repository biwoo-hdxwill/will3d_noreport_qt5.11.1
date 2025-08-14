#pragma once

#include <memory>

#include "../UIGLObjects/uiglobjects_defines.h"

class GLImplantWidget;
class CW3SurfaceItem;
class GLNerveWidget;
class BaseTransform;

class SurfaceItems {
public:
	SurfaceItems();
	~SurfaceItems();

	SurfaceItems(const SurfaceItems&) = delete;
	SurfaceItems& operator=(const SurfaceItems&) = delete;

	enum ItemType {
		NERVE = 0,
		IMPLANT,
		FACE,
		AIRWAY,
		ITEM_TYPE_END
	};

public:
	void InitItem(const ItemType& type, const UIGLObjects::ObjCoordSysType& coord_system_type);
	void SetVisible(const ItemType& type, bool is_visible);
	void SetSurfaceMVP(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p);
	void SetSurfaceMVP(const BaseTransform& transform);
	void SetSurfaceTransformMat(const ItemType& type, const glm::mat4& mat, const UIGLObjects::TransformType& transf_type);
	void ClearGL();

	void RenderAll(const unsigned int& program);
	void Render(const ItemType& type, const unsigned int& program);

	void RenderForPick(const ItemType & type, const unsigned int & program);

	glm::mat4 GetSurfaceTransformMat(const ItemType& type, const UIGLObjects::TransformType& transf_type);

	void ApplyPreferences();

private:
	void InitNerve(const UIGLObjects::ObjCoordSysType& coord_system_type);
	void InitImplant(const UIGLObjects::ObjCoordSysType& coord_system_type);
	void InitFace();
	void InitAirway();

	void PrintLogAndAssert(const char * msg);

private:
	std::unique_ptr<GLImplantWidget> implant_;
	std::unique_ptr<CW3SurfaceItem> face_;
	std::unique_ptr<GLNerveWidget> nerve_;
};
