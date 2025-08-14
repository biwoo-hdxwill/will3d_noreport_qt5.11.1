#include "view_pano_arch.h"

#include <QFont>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3MessageBox.h"

#include "../UIViewController/view_render_param.h"
#include "../UIViewController/view_controller_slice.h"

#include "../UIPrimitive/pano_arch_item.h"
#include "../UIPrimitive/W3TextItem.h"

#include "scene.h"

using namespace UIViewController;

ViewPanoArch::ViewPanoArch(QWidget* parent)
	: BaseViewPanoArch(parent) {
	txt_adjust_arch_.reset(new CW3TextItem());
	QFont old_font = txt_adjust_arch_->font();
	old_font.setPixelSize(old_font.pixelSize() + 3);
	txt_adjust_arch_->setFont(old_font);
	txt_adjust_arch_->setPlainText(lang::LanguagePack::txt_edit_arch());
	txt_adjust_arch_->setBackground(QColor(41, 45, 57));
	connect(txt_adjust_arch_.get(), SIGNAL(sigPressed()), this, SLOT(slotPressedAdjustArchText()));
	scene().addItem(txt_adjust_arch_.get());

	InitializeArchMenus();
}

ViewPanoArch::~ViewPanoArch() {}

/**=================================================================================================
public functions
*===============================================================================================**/
void ViewPanoArch::SetManualArchMode() {
  // 있으면 지워라
	if (arch()->IsFinished()) {
		arch()->Clear();
		emit sigDeletedArch();
	}

	if (txt_adjust_arch_->toPlainText() == lang::LanguagePack::txt_edit_arch())
		slotPressedAdjustArchText();

}
bool ViewPanoArch::SetApplyArch() {
	if (txt_adjust_arch_->toPlainText() == lang::LanguagePack::txt_apply_arch()) {
		return slotPressedAdjustArchText();
	} else return false;
}
void ViewPanoArch::HideAllUI(bool is_hide) {
	BaseViewPanoArch::HideAllUI(is_hide);
	txt_adjust_arch_->setVisible(!is_hide);
}
/**=================================================================================================
private slots
*===============================================================================================**/

void ViewPanoArch::slotDeleteArchFromQAction() {
	arch()->Clear();

	emit sigDeletedArch();
}

void ViewPanoArch::slotRemovePointArchFromQAction() {
	arch()->RemoveSelectedPoint();
}

void ViewPanoArch::slotInsertPointArchFromQAction() {
	arch()->InsertCloserPoint(pt_scene_current());
}
bool ViewPanoArch::slotPressedAdjustArchText() {
  if (txt_adjust_arch_->toPlainText() == lang::LanguagePack::txt_edit_arch()) {
	txt_adjust_arch_->setPlainText(lang::LanguagePack::txt_apply_arch());
	is_edit_mode_ = true;

	arch()->SetAdjustMode(is_edit_mode_);

	emit sigChangeEditArchMode();
	return true;
  } else if (txt_adjust_arch_->toPlainText() ==
			 lang::LanguagePack::txt_apply_arch()) {
	if (!arch()->IsFinished()) {
	  CW3MessageBox msg_box(QString("Will3D"), QString("Arch not created."),
							CW3MessageBox::Information);
	  msg_box.exec();
	  return false;
	} else {
	  txt_adjust_arch_->setPlainText(lang::LanguagePack::txt_edit_arch());
	  is_edit_mode_ = false;
	  this->UpdatedPanoRuler();
	  arch()->SetAdjustMode(is_edit_mode_);
	  emit sigChangeEditArchMode();
	  return true;
	}
  }
  return false;
}

void ViewPanoArch::slotEndEditArch() {
	BaseViewPanoArch::slotEndEditArch();

	if (is_edit_mode_) {
		slotPressedAdjustArchText();
	}
}
/**=================================================================================================
private functions
*===============================================================================================**/
void ViewPanoArch::InitializeArchMenus() {
	menu_arch_ell_.reset(new QMenu());
	menu_arch_spl_.reset(new QMenu());

	menu_act_delete_arch_.reset(new  QAction("Delete this arch"));
	menu_act_remove_pnt_arch_.reset(new  QAction("Remove the control point"));
	menu_act_insert_pnt_arch_.reset(new QAction("Insert the control point"));

	menu_arch_spl_->addAction(menu_act_insert_pnt_arch_.get());
	menu_arch_spl_->addAction(menu_act_delete_arch_.get());

	menu_arch_ell_->addAction(menu_act_remove_pnt_arch_.get());
	menu_arch_ell_->addAction(menu_act_delete_arch_.get());

	connect(menu_act_delete_arch_.get(), SIGNAL(triggered()), this, SLOT(slotDeleteArchFromQAction()));
	connect(menu_act_remove_pnt_arch_.get(), SIGNAL(triggered()), this, SLOT(slotRemovePointArchFromQAction()));
	connect(menu_act_insert_pnt_arch_.get(), SIGNAL(triggered()), this, SLOT(slotInsertPointArchFromQAction()));
}

bool ViewPanoArch::IsEventCancelLastArchPoint() const {
	if (controller()->IsReady() &&
		is_edit_mode_ &&
		!View::IsSetTool()) {
		return true;
	}
	return false;
}
void ViewPanoArch::resizeEvent(QResizeEvent * pEvent) {
	BaseViewPanoArch::resizeEvent(pEvent);

	const QSize& scene_size = view_render_param()->scene_size();
	QPointF begin_idx = mapToScene(QPoint(0, 0));

	txt_adjust_arch_->setPos(begin_idx + QPointF(30.0, 30.0));
}

void ViewPanoArch::mousePressEvent(QMouseEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (IsEventRightButton(event) && is_edit_mode_) 
	{
		if (arch()->IsHoveredLine()) 
		{
			menu_arch_spl_->popup(mapToGlobal(event->pos()) + QPoint(5, 5));
			QGraphicsView::mousePressEvent(event);
			return;
		}

		if (arch()->IsHoveredPoint()) 
		{
			arch()->SetSelectPointCurrentHover();
			menu_arch_ell_->popup(mapToGlobal(event->pos()) + QPoint(5, 5));
			QGraphicsView::mousePressEvent(event);
			return;
		}
	}

	BaseViewPanoArch::mousePressEvent(event);
}

void ViewPanoArch::mouseReleaseEvent(QMouseEvent* event) 
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

	if (View::IsSetTool()) 
	{
		BaseViewPanoArch::mouseReleaseEvent(event);
		return;
	}

	if (IsEventLeftButton(event) &&
		!txt_adjust_arch_->isUnderMouse() &&
		!arch()->IsFinished()) 
	{
		BaseViewPanoArch::AddPointArch(mapToScene(event->pos()));
	}

	BaseViewPanoArch::mouseReleaseEvent(event);
}

void ViewPanoArch::mouseDoubleClickEvent(QMouseEvent* event) 
{
	if (View::IsSetTool()) 
	{
		BaseViewPanoArch::mouseDoubleClickEvent(event);
		return;
	}

	if (IsEventLeftButton(event) && arch()->IsStartEdit()) 
	{
		BaseViewPanoArch::EndEditArch();
	}

	BaseViewPanoArch::mouseDoubleClickEvent(event);
}

void ViewPanoArch::mouseMoveEvent(QMouseEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (View::IsSetTool()) 
	{
		BaseViewPanoArch::mouseMoveEvent(event);
		return;
	}

	if (arch()->IsStartEdit()) 
	{
		this->DrawCurrentArch(mapToScene(event->pos()));
	}

	BaseViewPanoArch::mouseMoveEvent(event);
}

void ViewPanoArch::keyPressEvent(QKeyEvent* event) 
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
		if (IsEventCancelLastArchPoint())
		{
			arch()->CancelLastPoint();
		}
	}

	View::keyPressEvent(event);
}

void ViewPanoArch::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	BaseViewPanoArch::keyReleaseEvent(event);
}

void ViewPanoArch::DrawCurrentArch(const QPointF & pt_scene) 
{
	arch()->DrawingCurrentPath(pt_scene);
}
