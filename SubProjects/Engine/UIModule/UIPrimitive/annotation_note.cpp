#include "annotation_note.h"
#include <Engine/Resource/Resource/include/measure_data.h>
#include <Engine/Common/Common/W3Math.h>
#include "W3InputDialog.h"
#include "W3PathItem_anno.h"

AnnotationNote::AnnotationNote() {}

AnnotationNote::~AnnotationNote() {}

bool AnnotationNote::IsSelected() const {
	if (ui_line_->isSelected() || is_selected_)
		return true;
	else
		return false;
}

std::vector<QPointF> AnnotationNote::GetMeasurePoints() const {
	return std::vector<QPointF>{ui_line_->getCurveData()[0]};
}

void AnnotationNote::setSelected(bool bSelected) {
	is_selected_ = bSelected;
	setNodeDisplay(is_selected_);
}

void AnnotationNote::processMouseMove(const QPointF& pt, const glm::vec3& ptVol)
{
	if (ui_line_->isNodeSelected())
	{
		data_.lock().get()->set_points(ui_line_->getCurveData());
	}
}

void AnnotationNote::processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol) {
	input_dlg_.reset(new CW3InputDialog(CW3InputDialog::InputMode::TextInput));
	input_dlg_->setTitle("Edit Note");
	input_dlg_->setText(ui_line_->getLabel(0));
	input_dlg_->setFixedWidth(300);
	input_dlg_->raise();

	connect(input_dlg_.get(), SIGNAL(finished(int)), this, SLOT(slotFinishedNoteDlg()));
	input_dlg_->show();
}
void AnnotationNote::slotFinishedNoteDlg() { 
  QString inputText = input_dlg_->getInputText();
  if (inputText.length() < 1)
	inputText = ui_line_->getLabel(0);
  ui_line_->setLabel(inputText);

  emit sigSetNote(inputText);

  //disconnect(input_dlg_.get(), SIGNAL(finished(int)), this, SLOT(slotFinishedNoteDlg()));
  //input_dlg_.reset(nullptr);
}

bool AnnotationNote::InputParam(QGraphicsScene* pScene, const QPointF& pt,
								const glm::vec3& ptVol, bool& done) {
	ui_line_.reset(new CW3PathItem_anno(pScene));
	ui_line_->AddPoint(pt);
	ui_line_->setLabel(QString("Note - %1")
					   .arg(QString::number(data_.lock().get()->note_id())));
	ui_line_->EndEdit();
	done = true;

	processMouseDoubleClicked(pt, ptVol);
	return true;
}

void AnnotationNote::InputParamWithResource(QGraphicsScene * pScene, const QPointF& pt, const glm::vec3 & ptVol) {
	ui_line_.reset(new CW3PathItem_anno(pScene));
	ui_line_->AddPoint(pt);
	ui_line_->setLabel(data_.lock().get()->note_txt());
	ui_line_->EndEdit();
}

void AnnotationNote::setVisible(bool bShow) {
	ui_line_->setVisible(bShow);
}

QString AnnotationNote::NoteText() {
	if (ui_line_)
		return ui_line_->getLabel(0);
	return QString();
}

bool AnnotationNote::TransformItems(const QTransform & transform) {
	if (ui_line_) {
		const std::vector<QPointF> before = GetMeasurePoints();
		ui_line_->transformItems(transform);
		const std::vector<QPointF>& after = GetMeasurePoints();

		return W3::IsEqualPoint(before.at(0), after.at(0));
	}
	return false;
}

void AnnotationNote::ApplyPreferences() {
	if (ui_line_)
		ui_line_->ApplyPreferences();
}

QString AnnotationNote::GetValue() {
	return NoteText();
}
