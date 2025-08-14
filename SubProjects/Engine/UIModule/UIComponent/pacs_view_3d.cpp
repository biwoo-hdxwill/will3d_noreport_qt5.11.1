#include "pacs_view_3d.h"

#include <QTime>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Memory.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/nerve_resource.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../UIViewController/view_render_param.h"
#include "../UIViewController/view_controller_pacs_3d.h"

#include "scene.h"

using namespace UIViewController;

namespace
{
	const glm::vec3 kHorizontalAxis = glm::vec3(0.f, 0.f, -1.f);
	const glm::vec3 kVerticalAxis = glm::vec3(1.f, 0.f, 0.f);
}

PACSView3D::PACSView3D(QWidget* parent /*= 0*/)
	: View3D(common::ViewTypeID::PACS_3D, parent)
{
	controller_ = new ViewControllerPACS3D();
	controller_->set_view_param(view_render_param());

	view_render_param()->set_scene_scale(sqrt(2.0f));

	scene().InitViewItem(Viewitems::NAVIGATION);
	scene().SetMeasureReconType(common::ReconTypeID::VR);
}

PACSView3D::~PACSView3D()
{
	ClearGL();
	SAFE_DELETE_OBJECT(controller_);
}

/*=================================================================================================
public functions
=================================================================================================*/
void PACSView3D::SetVisibleNerve(bool is_visible)
{
	controller_->SetVisibleNerve(is_visible);
	RenderVolume();
	scene().update();
}

void PACSView3D::SetVisibleImplant(bool is_visible)
{
	controller_->SetVisibleImplant(is_visible);
	RenderVolume();
	scene().update();
}

void PACSView3D::ForceRotateMatrix(const glm::mat4& mat)
{
	controller_->ForceRotateMatrix(mat);
	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
	RenderVolume();
	scene().update();
}

void PACSView3D::EmitCreateDCMFile(const int cnt)
{
	QTime time = QTime::currentTime();
	QString middle_path = QString::number(time.msecsSinceStartOfDay()) + "_3D_";

	const CW3Image3D& vol_image = ResourceContainer::GetInstance()->GetMainVolume();
	int width = vol_image.width() * 2;
	int height = vol_image.height() * 2;
	int depth = vol_image.depth() * 2;

	int max = width > height ? width : height;
	max = max > depth ? max : depth;

	int num = num_;
	glm::vec3 axis;
	if (horizontal_)
	{
		axis = kHorizontalAxis;
	}
	else
	{
		axis = kVerticalAxis;
	}
	for (int i = 0; i < cnt; ++i)
	{
		float radian = glm::radians(static_cast<float>(angle_)) * static_cast<int>(num++);
		if (anterior_ == false)
		{
			radian += glm::pi<float>();
		}
		glm::mat4 rot_mat = glm::rotate(radian, axis);
		
		QString path = middle_path + QString::asprintf("%.4d.dcm", i);

		MakeCurrent();

		int size = max * max;
		unsigned char* buf = new unsigned char[size * 3];
		if (controller_->GetTextureData(buf, max, max, rot_mat))
		{
			emit sigCreateDCMFiles_uchar(buf, path, i, max, max);
		}
		delete[] buf;
		DoneCurrent();
	}
}

BaseViewController3D* PACSView3D::controller_3d()
{
	return static_cast<BaseViewController3D*>(controller_);
}

/*=================================================================================================
private functions
=================================================================================================*/
void PACSView3D::RenderVolume()
{
	if (!View::IsEnableGLContext())
	{
		return;
	}

	View::MakeCurrent();
	controller_->RenderingVolume();
	View::DoneCurrent();
}

void PACSView3D::drawBackground(QPainter* painter, const QRectF& rect)
{
	View::drawBackground(painter, rect);

	if (!controller_->IsReady())
	{
		return;
	}

	if (IsUpdateController())
	{
		controller_->RenderingVolume();
		UpdateDoneContoller();
	}

	painter->beginNativePainting();
	controller_->RenderScreen(View::GetDefaultFrameBufferObject());
	painter->endNativePainting();
}

void PACSView3D::resizeEvent(QResizeEvent* pEvent)
{
	View::resizeEvent(pEvent);
	controller_->SetProjection();

	RenderVolume();
}

void PACSView3D::InitializeController()
{
	controller_->Initialize();

	if (controller_->IsReady())
	{
		controller_->RenderingVolume();
	}

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
}

bool PACSView3D::IsReadyController()
{
	return controller_->IsReady();
}

void PACSView3D::ClearGL()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		controller_->ClearGL();
		View::DoneCurrent();
	}
}
