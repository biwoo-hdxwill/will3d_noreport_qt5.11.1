#include "view_pano_cross_section.h"

#include <QMouseEvent>

#include "../../Common/Common/color_will3d.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/cross_section_resource.h"
#include "../../Resource/Resource/nerve_resource.h"

#include "../UIPrimitive/W3EllipseItem.h"
#include "../UIViewController/view_controller_slice.h"

#include "scene.h"

using namespace UIViewController;

namespace
{
	const int kLineID = 0;
}

ViewPanoCrossSection::ViewPanoCrossSection(int cross_section_id, QWidget* parent)
	: BaseViewPanoCrossSection(cross_section_id, parent)
{
	ellipse_nerve_.reset(new CW3EllipseItem());
	ellipse_nerve_->SetFlagHighlight(true);
	ellipse_nerve_->SetFlagMovable(true);
	ellipse_nerve_->setVisible(false);
	connect(ellipse_nerve_.get(), SIGNAL(sigTranslatedEllipse(QPointF)), this, SLOT(slotTranslatedEllipse(QPointF)));

	scene().addItem(ellipse_nerve_.get());
}

ViewPanoCrossSection::~ViewPanoCrossSection()
{

}

/**=================================================================================================
public functions
*===============================================================================================**/
void ViewPanoCrossSection::SetEditNerveMode(const bool& is_edit)
{
	is_edit_nerve_mode_ = is_edit;
	ellipse_nerve_->SetFlagHighlight(is_edit);
	ellipse_nerve_->SetFlagMovable(is_edit);
}

void ViewPanoCrossSection::UpdateCrossSection()
{
	BaseViewPanoCrossSection::UpdateCrossSection();

	this->UpdateModifyNerveEll();
}

void ViewPanoCrossSection::UpdateModifyNerveEll()
{
	const auto& res_cross = ResourceContainer::GetInstance()->GetCrossSectionResource();
	ellipse_nerve_->setVisible(false);

	if ((&res_cross) == nullptr)
	{
		return;
	}

	const auto& cross_data = res_cross.data();
	auto iter_data = cross_data.find(cross_section_id());

	if (iter_data == cross_data.end())
	{
		return;
	}

	const auto& res_nerve = ResourceContainer::GetInstance()->GetNerveResource();
	const auto& modify_mode = res_nerve.modify_mode();
	QPointF pt_nerve;
	if (modify_mode.enable)
	{
		pt_nerve = iter_data->second->proj_nerve();
	}
	else
	{
		pt_nerve = iter_data->second->ctrl_nerve();
	}

	if (IsInCrossSectionPlane(pt_nerve.x(), pt_nerve.y()))
	{
		ellipse_nerve_->setPos(controller_slice()->MapPlaneToScene(pt_nerve));
		ellipse_nerve_->setPen(QPen(ColorNerveItem::kEllipsePenColor));
		ellipse_nerve_->setBrush(QBrush(ColorNerveItem::kEllipseBrushColor));
		ellipse_nerve_->setVisible(true);
		ellipse_nerve_->SetFlagMovable(modify_mode.enable);
		ellipse_nerve_->SetFlagHighlight(modify_mode.enable);
	}
}

#ifndef WILL3D_VIEWER
const int ViewPanoCrossSection::GetCrossSectionFilterLevel()
{
	return controller_slice()->GetSharpenLevel();
}

void ViewPanoCrossSection::RequestedCreateDCMFiles(const QString& time, bool nerve_visible, bool implant_visible, int filter, int thickness)
{
	CrossSectionResource::Params params = ResourceContainer::GetInstance()->GetCrossSectionResource().params();
	int width = params.width;
	int height = params.height;
	
	if (width == 0 || height == 0)
	{
		return;
	}
	
	QString middle_path = time + "CrossSection_";
	int id = cross_section_id();
	QString path = middle_path + QString::asprintf("%.4d.dcm", id);

	int size = width * height;
	if (!nerve_visible && !implant_visible)
	{
		unsigned short* buf = new unsigned short[size];
		MakeCurrent();
		bool is_ok = controller_slice()->GetTextureData(buf, width, height, filter, thickness);
		DoneCurrent();

		if (is_ok)
		{
			emit sigCreateDCMFiles_ushort(buf, path, id, height, width);
		}
		delete[] buf;
	}
	else
	{
		bool ori_nerve_visible = controller_slice()->IsNerveVisible();
		bool ori_implant_visible = controller_slice()->IsImplantVisible();

		controller_slice()->SetVisibleNerve(nerve_visible);
		controller_slice()->SetVisibleImplant(implant_visible);

		unsigned char* buf = new unsigned char[size * 3];
		MakeCurrent();
		bool is_ok = controller_slice()->GetTextureData(buf, width, height, filter, thickness);
		DoneCurrent();

		if (is_ok)
		{
			emit sigCreateDCMFiles_uchar(buf, path, id, height, width);
		}
		delete[] buf;

		controller_slice()->SetVisibleNerve(ori_nerve_visible);
		controller_slice()->SetVisibleImplant(ori_implant_visible);
	}
}
#endif

/**=================================================================================================
private functions
*===============================================================================================**/
void ViewPanoCrossSection::TransformItems(const QTransform& transform)
{
	BaseViewPanoCrossSection::TransformItems(transform);
	ellipse_nerve_->setPos(transform.map(ellipse_nerve_->pos()));
}

void ViewPanoCrossSection::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE
	BaseViewPanoCrossSection::mousePressEvent(event);
}

void ViewPanoCrossSection::mouseReleaseEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		if (event->button() == Qt::RightButton)
		{
			emit sigShowButtonListDialog(event->globalPos());
			return;
		}
	}
#endif // WILL3D_EUROPE

	if (IsEventLeftButton(event) && IsEventAddNerve())
	{
		QPointF pt_cross_section = controller_slice()->MapSceneToPlane(mapToScene(event->pos()));

		emit sigAddNerve(cross_section_id(), pt_cross_section);
	}

	is_pressed_double_click_ = false;

	BaseViewPanoCrossSection::mouseReleaseEvent(event);
}

void ViewPanoCrossSection::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	BaseViewPanoCrossSection::mouseMoveEvent(event);
}

void ViewPanoCrossSection::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (IsEventLeftButton(event) && IsEventEditNerve())
	{
		emit sigEndEditNerve();
	}
	else
	{
		BaseViewPanoCrossSection::mouseDoubleClickEvent(event);
	}

	is_pressed_double_click_ = true;
}

void ViewPanoCrossSection::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	if (event->key() == Qt::Key_Escape)
	{
		emit sigPressedKeyESC();
	}

	BaseViewPanoCrossSection::keyPressEvent(event);
}

void ViewPanoCrossSection::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
	}
#endif // WILL3D_EUROPE

	BaseViewPanoCrossSection::keyReleaseEvent(event);
}

bool ViewPanoCrossSection::IsEventAddNerve() const
{
	const auto& res_nerve = ResourceContainer::GetInstance()->GetNerveResource();
	const auto& modify_mode = res_nerve.modify_mode();
	return (is_edit_nerve_mode_ && !modify_mode.enable &&
		!is_pressed_double_click_ && !View::IsSetTool()) ? true : false;
}
bool ViewPanoCrossSection::IsEventEditNerve() const
{
	return (is_edit_nerve_mode_ && !View::IsSetTool()) ? true : false;
}

/**=================================================================================================
private slots
*===============================================================================================**/
void ViewPanoCrossSection::slotTranslatedEllipse(const QPointF& pos)
{
	if (!IsEventEditNerve())
	{
		return;
	}

	const auto& res_nerve = ResourceContainer::GetInstance()->GetNerveResource();
	const auto& modify_mode = res_nerve.modify_mode();
	if (!modify_mode.enable)
	{
		return;
	}

	QPointF pt_cross_plane = controller_slice()->MapSceneToPlane(pos);
	emit sigTranslatedNerve(cross_section_id(), pt_cross_plane);
}
