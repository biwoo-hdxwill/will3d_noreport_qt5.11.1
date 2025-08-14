#include "measure_profile.h"

#include <QDebug>

#include <Engine/Common/Common/language_pack.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Define.h>
#include <Engine/Common/Common/W3Math.h>
#include <Engine/Resource/Resource/include/measure_data.h>

#include "W3ProfileDialog.h"
#include "profile_line_item.h"

MeasureProfile::MeasureProfile() {
}

MeasureProfile::~MeasureProfile() {
}

void MeasureProfile::setSelected(bool bSelected) {
	is_selected_ = bSelected;
	setNodeDisplay(bSelected);

	if (is_selected_)
	{
		slotShowPlotter();
	}
}

bool MeasureProfile::IsSelected() const {
	if (is_drawing_)
		return false;

	if (ui_line_->isSelected() || is_selected_)
		return true;
	else
		return false;
}

void MeasureProfile::processMouseMove(const QPointF& pt, const glm::vec3& ptVol) 
{
	if (ui_line_->isSelected())
	{
		data_.lock().get()->set_points(ui_line_->getCurveData());
	}

	if (is_drawing_) 
	{
		ui_line_->drawingCurPath(pt);
	}
}

void MeasureProfile::processMouseReleased(const QPointF& pt, const glm::vec3& ptVol) {
	if (profile_dialog_ && is_drawing_ == false && node_count_ == 2) {
		slotShowAndUpdatePlotter();
	}
}

void MeasureProfile::slotPlotterWasClosed() {
	hideDialog();
}

bool MeasureProfile::InputParam(QGraphicsScene* pScene, const QPointF& pt,
									  const glm::vec3& ptVol, bool& done) {
	bool bProcessed = true;
	is_drawing_ = true;
	QString profile_name = QString("%1 - %2").arg(lang::LanguagePack::txt_profile()).arg(data_.lock().get()->profile_id());
	switch (node_count_) {
	case 0:
		ui_line_.reset(new ProfileLineItem(pScene));
		ui_line_->AddPoint(pt);
		ui_line_->drawingCurPath(pt);

		connect(ui_line_.get(), SIGNAL(sigMouseReleased()), this, SLOT(slotShowAndUpdatePlotter()));
		connect(ui_line_.get(), SIGNAL(sigLabelClicked()), this, SLOT(slotShowPlotter()));
		connect(ui_line_.get(), SIGNAL(sigTranslated()), this, SLOT(slotShowAndUpdatePlotter()));

		profile_dialog_.reset(new CW3ProfileDialog(profile_name, this));
		connect(profile_dialog_.get(), SIGNAL(sigPlotterWasClosed()), this, SLOT(slotPlotterWasClosed()));
		connect(profile_dialog_.get(), SIGNAL(sigChangeLengthStartPos(float)), ui_line_.get(), SLOT(slotChangeLengthStartPos(float)));
		connect(profile_dialog_.get(), SIGNAL(sigChangeLengthEndPos(float)), ui_line_.get(), SLOT(slotChangeLengthEndPos(float)));
		//connect(this, SIGNAL(exit()), m_pProfileDialog, SLOT(quit()));
		break;
	case 1:
		if (ui_line_->getCurveData().at(0) == pt)
			bProcessed = false;
		ui_line_->AddPoint(pt);
		ui_line_->setLabel(profile_name);
		ui_line_->EndEdit();
		is_drawing_ = false;
		done = true;
		slotShowAndUpdatePlotter();
		break;
	}


	++node_count_;
	return bProcessed;
}

void MeasureProfile::setNodeDisplay(bool bDisplay) {
	ui_line_->displayNode(bDisplay);

	if (!bDisplay) {
		hideDialog();
	}
}

void MeasureProfile::UpdateMeasure() {
	if (!ui_line_->IsVisible() || !profile_dialog_->isVisible())
	{
		return;
	}

	if (profile_dialog_.get() == nullptr) {
		QString profile_name = QString("%1 - %2").arg(lang::LanguagePack::txt_profile()).arg(data_.lock().get()->profile_id());
		profile_dialog_.reset(
			new CW3ProfileDialog(profile_name, this));
		//connect(this, SIGNAL(exit()), m_pProfileDialog, SLOT(quit()));
		connect(profile_dialog_.get(), SIGNAL(sigPlotterWasClosed()), this, SLOT(slotPlotterWasClosed()));
		connect(profile_dialog_.get(), SIGNAL(sigChangeLengthStartPos(float)), ui_line_.get(), SLOT(slotChangeLengthStartPos(float)));
		connect(profile_dialog_.get(), SIGNAL(sigChangeLengthEndPos(float)), ui_line_.get(), SLOT(slotChangeLengthEndPos(float)));
	}

	const auto& points = ui_line_->getCurveData();
	std::vector<short> data;
	emit sigGetPlotterData(points.at(0), points.at(1), data);
	SetPlotterData(data, points.at(0), points.at(1));

	if (profile_dialog_->isVisible())
	{
		profile_dialog_->raise();
	}
}

void MeasureProfile::slotShowAndUpdatePlotter() {
	if (ui_line_->getCurveData().size() < 2) {
		common::Logger::instance()->Print(common::LogType::DBG,
										  "CW3ProfileAnnotation::updatePlotter: Line is not set");
		return;
	}

	if (!ui_line_->IsVisible() ||
		!ui_line_->isSelected())
	{
		return;
	}

	slotShowPlotter();
	UpdateMeasure();
}

void MeasureProfile::slotShowPlotter() {
	profile_dialog_->show();
	profile_dialog_->raise();
}

void MeasureProfile::setVisible(bool bShow) {
	ui_line_->setVisible(bShow);

	if (!bShow)
	{
		hideDialog();
	}
}

std::vector<QPointF> MeasureProfile::GetMeasurePoints() const {
	if (ui_line_)
		return ui_line_->getCurveData();
	else
		return std::vector<QPointF>();
}

void MeasureProfile::SetPlotterData(const std::vector<short>& data,
										  const QPointF& start_pt, const QPointF& end_pt) {
	short min = std::numeric_limits<short>::max();
	short max = std::numeric_limits<short>::min();

	for (const auto& hu : data) {
		if (hu == common::dicom::kInvalidHU)
			continue;

		if (hu < min)
			min = hu;
		else if (hu > max)
			max = hu;
	}

#if 1
	QPointF p1 = start_pt;
	QPointF p2 = end_pt;
	QPointF fDist = p1 - p2;
	fDist.setX(fabs(fDist.rx()));
	fDist.setY(fabs(fDist.ry()));
	float pixel_dist = sqrt((float)(fDist.rx() * fDist.rx() + fDist.ry() * fDist.ry()));

	const auto& data_ptr = data_.lock().get();

	if (!data_ptr)
	{
		return;
	}

	float length = pixel_dist * data_ptr->pixel_pitch() * data_ptr->scale();
	float nearest = roundf(length * 100) / 100;
	ui_line_->setLabel(QString("%1 [mm]").arg(QString::number(nearest)));
#endif

	profile_dialog_->initialize(start_pt, end_pt, data,
								min, max, data_.lock().get()->pixel_pitch(), nearest);
}

bool MeasureProfile::TransformItems(const QTransform & transform) {
	if (ui_line_) {
		const std::vector<QPointF> before = GetMeasurePoints();
		ui_line_->transformItems(transform);
		const std::vector<QPointF>& after = GetMeasurePoints();
		return W3::IsEqualPoint(before.at(0), after.at(0));
	}
	return false;
}

void MeasureProfile::hideDialog() {
	if (!profile_dialog_)
	{
		return;
	}

	if (profile_dialog_->isVisible())
	{
		profile_dialog_->hide();
		profile_dialog_->lower();
	}
}

void MeasureProfile::ApplyPreferences() {
	if (ui_line_)
		ui_line_->ApplyPreferences();
}

QString MeasureProfile::GetValue() {
	short min, max;
	profile_dialog_->GetMinMax(min, max);
	return QString("Min : %1, Max : %2").arg(min).arg(max);
}
