#include "pacs_view_pano.h"

#include <QDir>
#include <QTime>

#include "../../Common/Common/W3Memory.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/pano_resource.h"

#include "../UIViewController/view_controller_pacs_image.h"

#include "scene.h"

using namespace UIViewController;

PACSViewPano::PACSViewPano(QWidget* parent /*= 0*/)
	: View(common::ViewTypeID::PANO, parent)
{
	controller_ = new ViewControllerPacsImage();
	controller_->set_view_param(view_render_param());
}

PACSViewPano::~PACSViewPano()
{
	if (IsEnableGLContext())
	{
		MakeCurrent();
		controller_->ClearGL();
		DoneCurrent();
	}

	SAFE_DELETE_OBJECT(controller_);
}

/*=================================================================================================
public functions
=================================================================================================*/
void PACSViewPano::InitPano(bool nerve, bool implant)
{
	if (init_pano_ == true)
	{
		return;
	}

	nerve_visible_ = nerve;
	implant_visible_ = implant;

	emit sigPanoUpdate(cur_pos_value_);
	SetPanoImage();

	init_pano_ = true;
}

void PACSViewPano::UpdatedPano()
{
	emit sigPanoUpdate(cur_pos_value_);
	controller_->ImageUpdate();
	SetPanoImage();

	RenderBackScreen();
	scene().update();
}

void PACSViewPano::EmitCreateDCMFile(const int range)
{
	QString middle_path = CreateDicomFileMiddlePath();

	for (int i = 0; i < range; ++i)
	{
		emit sigPanoUpdate(cur_pos_value_ + i);
		controller_->ImageUpdate();
		SetPanoImage();

		RenderBackScreen();

		QString path = middle_path + QString::asprintf("%.4d.dcm", i);

		int width, height;
		if (nerve_visible_ || implant_visible_)
		{
			unsigned char* buf = nullptr;
			MakeCurrent();
			bool is_ok = controller_->GetTextureData(buf, width, height);
			DoneCurrent();
			
			if (is_ok)
			{
				emit sigCreateDCMFiles_uchar(buf, path, i, height, width);
			}
			
			if (buf)
			{
				delete[] buf;
			}
		}
		else
		{
			unsigned short* buf = nullptr;
			MakeCurrent();
			bool is_ok = controller_->GetTextureData(buf, width, height);
			DoneCurrent();

			if (is_ok)
			{
				emit sigCreateDCMFiles_ushort(buf, path, i, height, width);
			}
			
			if (buf)
			{
				delete[] buf;
			}
		}
	}
}

void PACSViewPano::SetCurPosValue(const int value)
{
	if (value == cur_pos_value_)
	{
		return;
	}

	cur_pos_value_ = value;

	if (init_pano_)
	{
		UpdatedPano();
	}
}

void PACSViewPano::SetNerveVisibility(bool is_visible)
{
	if (nerve_visible_ != is_visible)
	{
		nerve_visible_ = is_visible;
		RenderBackScreen();
		scene().update();
	}
}

void PACSViewPano::SetImplantVisibility(bool is_visible)
{
	if (implant_visible_ != is_visible)
	{
		implant_visible_ = is_visible;
		RenderBackScreen();
		scene().update();
	}
}

void PACSViewPano::SetSharpenLevel(const int level)
{
	SharpenLevel next_level = static_cast<SharpenLevel>(level);
	SharpenLevel cur_level = controller_->sharpen_level();
	if (cur_level != next_level)
	{
		controller_->SetSharpenLevel(next_level);
		RenderBackScreen();
		scene().update();
	}
}

void PACSViewPano::ApplyPreferences()
{
	View::ApplyPreferences();
}

/**=================================================================================================
private functions
*===============================================================================================**/
void PACSViewPano::SetPanoImage()
{
	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	if (&res_pano == nullptr)
	{
		return;
	}

	controller_->SetImage(res_pano.GetPanoImageWeakPtr(), GL_TEXTURE_HANDLE::TEX_IMAGE);
	controller_->SetImage(res_pano.GetMaskNerveImageWeakPtr(), GL_TEXTURE_HANDLE::TEX_NERVE_MASK);
	controller_->SetImage(res_pano.GetMaskimplantImageWeakPtr(), GL_TEXTURE_HANDLE::TEX_IMPLANT_MASK);
}

void PACSViewPano::RenderBackScreen()
{
	MakeCurrent();
	controller_->RenderBackScreen(nerve_visible_, implant_visible_);
	DoneCurrent();	
}

void PACSViewPano::ClearGL()
{
	controller_->ClearGL();
}

void PACSViewPano::resizeEvent(QResizeEvent* pEvent)
{
	View::resizeEvent(pEvent);
	if (!view_render_param() || !is_view_ready())
	{
		return;
	}

	controller_->SetProjection();
}

void PACSViewPano::drawBackground(QPainter* painter, const QRectF& rect)
{
	QGraphicsView::drawBackground(painter, rect);

	if (IsUpdateController())
	{
		if (cur_pos_value_ != -1)
		{
			controller_->RenderBackScreen(nerve_visible_, implant_visible_);
		}

		UpdateDoneContoller();
	}

	painter->beginNativePainting();
	controller_->RenderScreen(GetDefaultFrameBufferObject());
	painter->endNativePainting();
}

QString PACSViewPano::CreateDicomFileMiddlePath()
{
	QTime time = QTime::currentTime();
	QString middle_path = QString::number(time.msecsSinceStartOfDay()) + "_Pano_";

	return middle_path;
}
