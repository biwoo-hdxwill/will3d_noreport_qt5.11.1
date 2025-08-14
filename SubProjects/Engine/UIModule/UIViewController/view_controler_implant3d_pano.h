#pragma once

#include "view_controller_pano3d.h"

class CW3SurfaceAxesItem;
class ImplantData;

class UIVIEWCONTROLLER_EXPORT ViewControllerImplant3Dpano : public ViewControllerPano3D {
public:
	explicit ViewControllerImplant3Dpano();
	~ViewControllerImplant3Dpano();

	ViewControllerImplant3Dpano(const ViewControllerImplant3Dpano&) = delete;
	ViewControllerImplant3Dpano& operator=(const ViewControllerImplant3Dpano&) = delete;

public:

	void MoveImplant(int* implant_id,
					 glm::vec3* delta_translate,
					 glm::vec3* rotate_axes,
					 float* delta_degree);
	void PickAxesItem(bool* is_update_scene);
	void RenderForPickAxes();

	virtual void ClearGL() override;
	void SelectImplant(int* implant_id);
	bool IsPickImplant() const;
	int GetPickImplantID() const;

	inline const bool is_selected_implant() const { return is_selected_implant_; }
private:
	ImplantData* GetImplantData() const;
	virtual void SetSurfaceMVP() override;
	void SetSurfaceAxesMVP();
	virtual void DrawOverwriteSurface() override;

	virtual void SetPackTexture() override;
	virtual void SetPackTextureHandle() override;
	virtual void ReadyBufferHandles() override;

private:
	enum GL_TEXTURE_HANDLE_IMP {
		TEX_PICK_IMPLANT = GL_TEXTURE_HANDLE::TEX_END,
		TEX_PICK_AXIS,
		TEX_IMP_END
	};

	PackTexture pack_pick_implant_;
	PackTexture pack_pick_axes_;

	std::unique_ptr<CW3SurfaceAxesItem> axes_item_;

	bool is_selected_implant_ = false;
	int id_axes_implant_ = -1;
};
