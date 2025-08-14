#include "pacs_view_mpr.h"

#include <QDebug>
#include <QOpenGLWidget>

#include <QDir>
#include <QTime>
#include <QDirIterator>

#include "../../Common/Common/W3Math.h"
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/global_preferences.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../UIViewController/pacs_view_controller.h"
#include "../UIViewController/view_render_param.h"

#include "scene.h"

using namespace UIViewController;

namespace
{
	const glm::vec3 kX_Inverse = glm::vec3(-1.f, 1.f, 1.f);
	static int cnt = 0;
}

PACSViewMPR::PACSViewMPR(QWidget* parent /*= 0*/)
	: View(common::ViewTypeID::PACS_MPR, parent)
{
	if (controller_ == nullptr)
	{
		controller_ = new PACSViewController();
		controller_->set_is_implant_wire(true);
		controller_->set_view_param(View::view_render_param());
	}

	scene().InitViewItem(Viewitems::NAVIGATION);
	scene().InitMeasure(view_type());

	GlobalPreferences::PACSDefaultSetting& setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;
	thickness_ = setting.mpr_thickness;
}

PACSViewMPR::~PACSViewMPR()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		ClearGL();
		SAFE_DELETE_OBJECT(controller_);
		View::DoneCurrent();
	}
}

/**=================================================================================================
public functions
*===============================================================================================**/
void PACSViewMPR::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	View::SetCommonToolOnOff(type);
}

void PACSViewMPR::UpdateSlice()
{
	this->RenderSlice();
	scene().update();
}

void PACSViewMPR::UpdateTranslationSliderValue(const int value)
{
	slider_value_ = value;
	UpdateController();
	scene().update();
}

void PACSViewMPR::UpdateRotationSliderValue(const int value, bool is_rot_horizontal)
{
	slider_value_ = value;
	is_rot_horizontal_ = is_rot_horizontal;

	UpdateRotataionController();
	scene().update();
}

void PACSViewMPR::UpdateThickness(const int thickness, bool is_trans)
{
	if (thickness_ == thickness)
	{
		return;
	}

	thickness_ = thickness;
	if (is_trans)
	{
		UpdateController();
	}
	else
	{
		UpdateRotataionController();
	}
	scene().update();
}

void PACSViewMPR::UpdateFilter(const int filter)
{
	ActiveSharpen(filter);
}

void PACSViewMPR::UpdateRotation(const bool is_rot_horizontal)
{
	if (is_rot_horizontal_ == is_rot_horizontal)
	{
		return;
	}

	is_rot_horizontal_ = is_rot_horizontal;
	UpdateRotataionController();
	scene().update();
}

void PACSViewMPR::SetVisibleNerve(bool is_visible)
{
	controller_->SetVisibleNerve(is_visible);
	scene().update();
}

void PACSViewMPR::SetVisibleImplant(bool is_visible)
{
	controller_->SetVisibleImplant(is_visible);
	scene().update();
}

void PACSViewMPR::EmitCreateDCMFile(const int range, const bool is_translation, const bool draw_surface)
{
	if (renewal_width_ == 0 || renewal_height_ == 0)
	{
		return;
	}

	QString middle_path = CreateDicomFileMiddlePath();
	int ori_value = slider_value_;

	int width = renewal_width_;
	int height = renewal_height_;

	MakeCurrent();
	controller_->InitOffScreenBuff(width, height);
	DoneCurrent();

	for (int i = 0; i < range; ++i)
	{
		if (is_translation)
		{
			UpdateController();
		}
		else
		{
			UpdateRotataionController();
		}

		if (width != renewal_width_ || height != renewal_height_)
		{
			width = renewal_width_;
			height = renewal_height_;

			MakeCurrent();
			controller_->UpdateOffFrameBuffer(width, height);
			DoneCurrent();
		}

		++slider_value_;

		QString path = middle_path + QString::asprintf("%.4d.dcm", i);

		MakeCurrent();
		int size = width * height;
		if (draw_surface)
		{
			unsigned char* buf = new unsigned char[size * 3];
			if (controller_->GetTextureData(buf, width, height))
			{
				emit sigCreateDCMFiles_uchar(buf, path, i, height, width);
			}
			delete[] buf;
		}
		else
		{
			unsigned short* buf = new unsigned short[size];
			if (controller_->GetTextureData(buf, width, height))
			{
				emit sigCreateDCMFiles_ushort(buf, path, i, height, width);
			}
			delete[] buf;
		}
		DoneCurrent();
	}

	slider_value_ = ori_value;
	if (is_translation)
	{
		UpdateController();
	}
	else
	{
		UpdateRotataionController();
	}
}

/**=================================================================================================
protected functions
*===============================================================================================**/
void PACSViewMPR::resizeEvent(QResizeEvent* pEvent)
{
	View::resizeEvent(pEvent);

	controller_->SetProjection();
}

void PACSViewMPR::RenderSlice()
{
	if (!View::IsEnableGLContext())
	{
		return;
	}

	MakeCurrent();
	controller_->RenderingSlice();
	DoneCurrent();
}

/**=================================================================================================
private functions
*===============================================================================================**/
void PACSViewMPR::drawBackground(QPainter* painter, const QRectF& rect)
{
	View::drawBackground(painter, rect);

	if (IsReadyController())
	{
		painter->beginNativePainting();
		controller_->RenderingSlice();
		controller_->RenderScreen(View::GetDefaultFrameBufferObject());
		painter->endNativePainting();
	}
}

void PACSViewMPR::InitializeController()
{
	controller_->Initialize();

	UpdateController();
}

bool PACSViewMPR::IsReadyController()
{
	return controller_->IsReady();
}

void PACSViewMPR::ClearGL()
{
	controller_->ClearGL();
}

void PACSViewMPR::ActiveControllerViewEvent()
{
	/*bool need_render = false;
	controller_->ProcessViewEvent(&need_render);

	if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT)
	{
		need_render = true;
	}

	if (need_render)
	{
		RenderSlice();
	}*/
}

void PACSViewMPR::UpdateController()
{
	RenewalWH(plane_right_, plane_back_);

	const CW3Image3D& main_volume = ResourceContainer::GetInstance()->GetMainVolume();
	float vol_width = main_volume.width();
	float vol_height = main_volume.height();
	float vol_depth = main_volume.depth();

	glm::vec3 vol_range = glm::vec3(vol_width, vol_height, vol_depth);
	glm::vec3 vol_center = (vol_range - glm::vec3(1.f)) * 0.5f;

	glm::vec3 up = glm::normalize(glm::cross(plane_back_, plane_right_));
	if (is_inverse_up_vec_ == false)
	{
		up = -up;
	}

	glm::vec3 min = vol_center - up * static_cast<float>(available_depth_);
	min = W3::FitVolRange(vol_range, vol_center, min);

	glm::vec3 off_vec = glm::vec3(0.f);
	if (offset_ != 0)
	{
		off_vec = up * static_cast<float>(offset_);
	}

	glm::vec3 mapping_slider_vec = up * static_cast<float>(slider_value_ * spacing_ratio_);
	glm::vec3 pos = min + mapping_slider_vec + off_vec;

	controller_->SetPlane(pos,
		static_cast<float>(renewal_width_) * plane_right_,
		static_cast<float>(renewal_height_) * plane_back_,
		thickness_);

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
}

void PACSViewMPR::UpdateRotataionController()
{
	glm::vec3 right, back, up;

	if (slider_value_ != 0)
	{
		glm::mat4 rot_mat = glm::mat4(1.f);
		float result_angle = rotation_angle_ * slider_value_;

		if (is_rot_horizontal_)
		{
			rot_mat = glm::rotate(result_angle, plane_back_);
		}
		else
		{
			rot_mat = glm::rotate(result_angle, plane_right_);
		}

		right = glm::normalize(glm::vec3(rot_mat * glm::vec4(plane_right_, 1.f)));
		back = glm::normalize(glm::vec3(rot_mat * glm::vec4(plane_back_, 1.f)));
	}
	else
	{
		right = plane_right_;
		back = plane_back_;
	}

	up = glm::normalize(glm::cross(back, right));

	if (is_inverse_up_vec_ == false)
	{
		up = -up;
	}

	const CW3Image3D& main_volume = ResourceContainer::GetInstance()->GetMainVolume();
	float vol_width = main_volume.width();
	float vol_height = main_volume.height();
	float vol_depth = main_volume.depth();

	glm::vec3 vol_range = glm::vec3(vol_width, vol_height, vol_depth);
	glm::vec3 vol_center = (vol_range - glm::vec3(1.f)) * 0.5f;

	RenewalWH(right, back);

	glm::vec3 direction = cross_pos_ - vol_center;
	glm::vec3 destination = glm::vec3(0.f);
	float direction_length = glm::length(direction);

	if (direction_length > glm::epsilon<float>())
	{
		float length = glm::dot(direction, up);
		destination = up * length;
	}

	controller_->SetPlane(vol_center + destination,
		static_cast<float>(renewal_width_) * right,
		static_cast<float>(renewal_height_) * back,
		thickness_);

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
}

void PACSViewMPR::ActiveSharpen(const int index)
{
	SharpenLevel sharpen_level = static_cast<SharpenLevel>(index);
	if (controller_->sharpen_level() != sharpen_level)
	{
		controller_->set_sharpen_level(sharpen_level);

		scene().update();
	}
}

void PACSViewMPR::RenewalWH(const glm::vec3& right, const glm::vec3& back)
{
	const CW3Image3D& main_volume = ResourceContainer::GetInstance()->GetMainVolume();
	float vol_width = main_volume.width();
	float vol_height = main_volume.height();
	float vol_depth = main_volume.depth();

	glm::vec3 vol_range = glm::vec3(vol_width, vol_height, vol_depth);
	glm::vec3 vol_center = (vol_range - glm::vec3(1.f)) * 0.5f;

	std::vector<glm::vec3> vol_vertex = controller_->GetVolVertex();

	int new_width = 0;
	int	new_height = 0;
	int temp_value = 0;
	for (int i = 0; i < 8; ++i)
	{
		glm::vec3 vec = vol_vertex[i] - vol_center;

		temp_value = glm::dot(vec, right);
		new_width = std::max(new_width, std::abs(temp_value));

		temp_value = glm::dot(vec, back);
		new_height = std::max(new_height, std::abs(temp_value));
	}

	renewal_width_ = new_width * 2;
	renewal_height_ = new_height * 2;
}

QString PACSViewMPR::CreateDicomFileMiddlePath()
{
	QTime time = QTime::currentTime();
	QString middle_path = QString::number(time.msecsSinceStartOfDay()) + "_";
	if (mpr_type_ == MPRViewType::AXIAL)
	{
		middle_path += "Axial_";
	}
	else if (mpr_type_ == MPRViewType::SAGITTAL)
	{
		middle_path += "Sagittal_";
	}
	else if (mpr_type_ == MPRViewType::CORONAL)
	{
		middle_path += "Coronal_";
	}

	return middle_path;
}
