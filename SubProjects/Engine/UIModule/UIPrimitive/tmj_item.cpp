#include "tmj_item.h"

#include <qmath.h>
#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/common.h"

#include "../../UIPrimitive/W3ArcItem.h"
#include "../../UIPrimitive/W3Curve.h"
#include "../../UIPrimitive/W3EllipseItem.h"
#include "../../UIPrimitive/W3LineItem.h"
#include "../../UIPrimitive/polygon_item.h"
#include "../../UIPrimitive/tmj_arrow_item.h"

#include "W3TextItem.h"
#include "line_list_item.h"

namespace
{
	const double kLineWidth = 1.0;

	const double kZorderROI = 1.0;
	const double kZorderHLine = 2.0;
	const double kZorderVLine = 3.0;
	const double kZorderArrow = 4.0;
	const double kZorderEllipse = 5.0;

	const float kAvailableHeight = 30.0f;
	const float kAngleLineLength = 85.0f;

	auto VtoQPointF = [](const QVector2D& v) -> QPointF
	{
		return QPointF((double)v.x(), (double)v.y());
	};
}  // namespace

TMJItem::TMJItem()
{
	this->setAcceptHoverEvents(true);
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);

	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		this->CreateTMJwidget((TMJDirectionType)i);
		this->CreateAngleWidget((TMJDirectionType)i);
	}
	this->CreateCriterionLine();
}

TMJItem::~TMJItem() {}

QRectF TMJItem::boundingRect() const
{
	QPointF left_top(std::numeric_limits<double>::max(),
		std::numeric_limits<double>::max());
	QPointF right_bot(std::numeric_limits<double>::min(),
		std::numeric_limits<double>::min());

	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		if (tmj_widget_[i].roi == nullptr) continue;

		auto points = tmj_widget_[i].roi->polygon().toList();
		for (const auto& pt : points)
		{
			left_top.setX(std::min(pt.x(), left_top.x()));
			left_top.setY(std::min(pt.y(), left_top.y()));
			right_bot.setX(std::max(pt.x(), right_bot.x()));
			right_bot.setY(std::max(pt.y(), right_bot.y()));
		}
	}

	return QRectF(left_top.x(), left_top.y(), right_bot.x() - left_top.x(),
		right_bot.y() - left_top.y());
}

/*=============================================================================================
public functions
===============================================================================================*/

void TMJItem::Initialize(const InitParam& params)
{
	roi_info_[TMJ_LEFT].width = params.left_roi_width;
	roi_info_[TMJ_LEFT].height = params.left_roi_height;
	roi_info_[TMJ_RIGHT].width = params.right_roi_width;
	roi_info_[TMJ_RIGHT].height = params.right_roi_height;

	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		QPointF pt_scene_center(params.scene_width * 0.5f,
			params.scene_height * 0.5f);
		QPointF pt_roi_center;

		QTransform rotate_mat;
		if (i == TMJ_LEFT)
		{
			pt_roi_center = pt_scene_center + QPointF(params.scene_width * 0.15f,
				params.scene_height * 0.15f);
			rotate_mat.translate(pt_roi_center.x(), pt_roi_center.y());
			rotate_mat.rotate(-params.degree_angle);
			rotate_mat.translate(-pt_roi_center.x(), -pt_roi_center.y());
		}
		else if (i == TMJ_RIGHT)
		{
			pt_roi_center = pt_scene_center + QPointF(-params.scene_width * 0.15f,
				params.scene_height * 0.15f);
			rotate_mat.translate(pt_roi_center.x(), pt_roi_center.y());
			rotate_mat.rotate(params.degree_angle);
			rotate_mat.translate(-pt_roi_center.x(), -pt_roi_center.y());
		}
		else
		{
			return;
		}

		QPointF p1 = rotate_mat.map(pt_roi_center +
			QPointF((double)roi_info_[i].width * 0.5, 0.0));
		QPointF p2 = rotate_mat.map(pt_roi_center -
			QPointF((double)roi_info_[i].width * 0.5, 0.0));

		roi_info_[i].center = (p1 + p2) * 0.5;

		tmj_widget_[i].frontal_ellipse[0]->setPos(p1);
		tmj_widget_[i].frontal_ellipse[1]->setPos(p2);
	}

	angle_length_ = kAngleLineLength;

	float crit_offset = params.scene_height * 0.1f;
	criterion_line_->addPoint(
		QPointF((double)params.scene_width * 0.5, (double)crit_offset));
	criterion_line_->addPoint(QPointF((double)params.scene_width * 0.5,
		(double)(params.scene_height - crit_offset)));

	criterion_line_->endEdit();
	criterion_line_->drawCurve();

	is_init = true;

	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		UpdateItems((TMJDirectionType)i);
	}
}
void TMJItem::SetPositionAndDegree(const TMJDirectionType& type,
	const QPointF& center_pos,
	const QVector2D& up_vector)
{
	roi_info_[type].center = center_pos;
	QPointF p1 =
		center_pos + VtoQPointF(roi_info_[type].width * 0.5f * up_vector);
	QPointF p2 =
		center_pos - VtoQPointF(roi_info_[type].width * 0.5f * up_vector);
	tmj_widget_[type].frontal_ellipse[0]->setPos(p1);
	tmj_widget_[type].frontal_ellipse[1]->setPos(p2);
	UpdateItems(type);
}
void TMJItem::SetFrontalLineVisible(const TMJDirectionType& type,
	const bool& is_visible)
{
	roi_info_[type].is_frontal_visible_ = is_visible;

	if (!IsValidTMJ(type))
	{
		return;
	}
	tmj_widget_[type].frontal_line->setVisible(is_visible);
	tmj_widget_[type].frontal_ellipse[0]->setVisible(is_visible);
	tmj_widget_[type].frontal_ellipse[1]->setVisible(is_visible);
}

void TMJItem::SetLateralLineVisible(const TMJDirectionType& type,
	const bool& is_visible)
{
	roi_info_[type].is_lateral_visible_ = is_visible;

	if (!IsValidTMJ(type))
	{
		return;
	}

	tmj_widget_[type].lateral_line->setVisible(is_visible);
}

void TMJItem::SetHighlightLateralLine(const TMJDirectionType& type,
	const int& index)
{
	roi_info_[type].lateral_highlight_index = index;
	UpdateHighlightLateralLine(type);
}

void TMJItem::SetTMJRectSize(const TMJRectID& roi_id, float value)
{
	TMJDirectionType direction_type;
	switch (roi_id)
	{
	case TMJRectID::LEFT_H:
		roi_info_[TMJDirectionType::TMJ_LEFT].height = value;
		direction_type = TMJDirectionType::TMJ_LEFT;
		break;
	case TMJRectID::LEFT_W:
		roi_info_[TMJDirectionType::TMJ_LEFT].width = value;
		direction_type = TMJDirectionType::TMJ_LEFT;
		break;
	case TMJRectID::RIGHT_H:
		roi_info_[TMJDirectionType::TMJ_RIGHT].height = value;
		direction_type = TMJDirectionType::TMJ_RIGHT;
		break;
	case TMJRectID::RIGHT_W:
		roi_info_[TMJDirectionType::TMJ_RIGHT].width = value;
		direction_type = TMJDirectionType::TMJ_RIGHT;
		break;
	default:
		break;
	}

	if (IsValidTMJ(direction_type)) UpdateItems(direction_type);
}

void TMJItem::GetROIRectSize(const TMJDirectionType& type, float* width,
	float* height) const
{
	if (!IsValidTMJ(type))
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR, "TMJItem::GetLateralPositionInfo: Invalid TMJ.");
		return;
	}

	*width = roi_info_[type].width;
	*height = roi_info_[type].height;
}

bool TMJItem::ShiftLateralLine(const TMJDirectionType& type,
	const float& value)
{
	if (!IsValidTMJ(type))
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, "TMJItem::ShifteLateralLine: Invalid TMJ.");
		return false;
	}
	if (value == 0.0f)
	{
		return false;
	}

	bool shiftable = IsShiftableLateralLine(type, value);
	if (shiftable)
	{
		roi_info_[type].lateral_line_shifted += value;
		ClampShiftedLateralLine(type);
		UpdateItems(type);
	}

	return shiftable;
}

bool TMJItem::ShiftFrontalLine(const TMJDirectionType& type,
	const float& value)
{
	if (!IsValidTMJ(type))
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, "TMJItem::ShiftFrontalLine: Invalid TMJ.");
		return false;
	}
	if (value == 0.0f)
	{
		return false;
	}

	bool shiftable = IsShiftableFrontalLine(type, value);
	if (shiftable)
	{
		roi_info_[type].frontal_line_shifted += value;
		ClampShiftedFrontalLine(type);
		UpdateItems(type);
	}

	return shiftable;
}

bool TMJItem::GetROIcenter(const TMJDirectionType& type,
	QPointF* pt_center_in_scene)
{
	if (!IsValidTMJ(type))
	{
		return false;
	}

	*pt_center_in_scene = roi_info_[type].center;
	return true;
}
bool TMJItem::GetLateralPositionInfo(const TMJDirectionType& type,
	std::map<float, QPointF>* pt_center_in_scene,
	QPointF* up_vector_in_scene) const
{
	if (!IsValidTMJ(type))
	{
		return false;
	}

	const auto& positions = tmj_widget_[type].lateral_line->line_positions();

	if (positions.size() == 0) return false;

	std::map<float, QPointF> line_pos;
	for (int i = 0; i < positions.size(); i++)
	{
		if ((int)roi_info_[type].line_id.size() <= i)
		{
			assert(false);
			continue;
		}

		float line_id = (float)roi_info_[type].line_id[i];
		auto iter = positions.find(i);
		if (iter == positions.end()) assert(false);
		line_pos[line_id] = iter->second;
	}

	*pt_center_in_scene = line_pos;

	QVector2D up_vector;
	GetFrontalVector(type, &up_vector);
	*up_vector_in_scene = VtoQPointF(up_vector);
	return true;
}

const float& TMJItem::GetLateralInterval(const TMJDirectionType& type) const
{
	return roi_info_[type].lateral_line_interval;
	// TODO: 여기에 반환 구문을 삽입합니다.
}

bool TMJItem::GetFrontalPositionInfo(const TMJDirectionType& type,
	QPointF* pt_center_in_scene,
	QPointF* up_vector_in_scene) const
{
	if (!IsValidTMJ(type))
	{
		return false;
	}

	const auto& positions = tmj_widget_[type].frontal_line->line_positions();

	if (positions.size() != 1)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJItem::GetFrontalPositionInfo: please check frontal line.");
		return false;
	}

	*pt_center_in_scene = positions.begin()->second;

	QVector2D up_vector;
	GetLateralVector(type, &up_vector);
	*up_vector_in_scene = VtoQPointF(up_vector);
	return true;
}

void TMJItem::AddPoint(const QPointF& pt, TMJDirectionType* tmj_type_created)
{
	TMJDirectionType draw_type = this->GetDrawType();
	*tmj_type_created = draw_type;

	if (draw_type == TMJ_TYPE_UNKNOWN)
	{
		return;
	}

	if (draw_line_ == nullptr)
	{
		CreateDrawLine();
	}

	if (draw_line_->getCurveData().size() > 1)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJItem::AddPoint: point's count is greater than one.");
	}

	draw_line_->addPoint(pt);

	if (draw_line_->getCurveData().size() == 2)
	{
		CreateTMJwidget(draw_type);
		CreateAngleWidget(draw_type);

		draw_line_->sortPointsOrderX();
		QPointF p1 = draw_line_->getCurveData()[0];
		QPointF p2 = draw_line_->getCurveData()[1];

		roi_info_[draw_type].center = (p1 + p2) * 0.5;
		roi_info_[draw_type].width = QVector2D(p2 - p1).length();
		tmj_widget_[draw_type].frontal_ellipse[0]->setPos(p2);
		tmj_widget_[draw_type].frontal_ellipse[1]->setPos(p1);

		DeleteDrawLine();
		UpdateItems(draw_type);

		*tmj_type_created = draw_type;
		draw_on_[draw_type] = false;
	}
}

void TMJItem::DrawPoint(const QPointF& pt)
{
	TMJDirectionType draw_type = this->GetDrawType();
	if (draw_type == TMJ_TYPE_UNKNOWN)
	{
		return;
	}

	if (draw_line_->getCurveData().size() != 1)
	{
		return;
	}

	draw_line_->drawingCurPath(pt);
}

void TMJItem::SetDrawOn(const TMJDirectionType& direction_type, bool draw_on)
{
	draw_on_[direction_type] = draw_on;
}

void TMJItem::ClearTMJ(const TMJDirectionType& type)
{
	this->DeleteTMJwidget(type);
	this->DeleteAngleWidget(type);
	emit sigUpdated(type);
}

void TMJItem::TransformItems(const QTransform& transform)
{
	criterion_line_->TransformItems(transform);

	float scale = std::max(transform.m11(), transform.m22());
	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		if (!IsValidTMJ((TMJDirectionType)i)) continue;

		tmj_widget_[i].roi->TransformItems(transform);
		tmj_widget_[i].frontal_line->TransformItems(transform);
		tmj_widget_[i].lateral_line->TransformItems(transform);
		for (int k = 0; k < 2; k++)
		{
			tmj_widget_[i].frontal_ellipse[k]->setPos(
				transform.map(tmj_widget_[i].frontal_ellipse[k]->pos()));
			tmj_widget_[i].lateral_resize_arrow[k]->TransformItems(transform);
			tmj_widget_[i].frontal_resize_arrow[k]->TransformItems(transform);
		}

		roi_info_[i].center = transform.map(roi_info_[i].center);
		roi_info_[i].width *= scale;
		roi_info_[i].height *= scale;
		roi_info_[i].lateral_line_interval *= scale;
		roi_info_[i].lateral_line_shifted *= scale;
		roi_info_[i].frontal_line_shifted *= scale;
		for (auto& elem : roi_info_[i].line_id)
		{
			elem *= scale;
		}
		UpdateAngleItems((TMJDirectionType)i);
	}
}
void TMJItem::SetLateralParam(const TMJLateralID& id, const float& value)
{
	switch (id)
	{
	case LEFT_INTERVAL:
		roi_info_[TMJ_LEFT].lateral_line_interval = value;
		ClampShiftedLateralLine(TMJDirectionType::TMJ_LEFT);
		UpdateItems(TMJ_LEFT);
		break;
	case LEFT_THICKNESS:
		roi_info_[TMJ_LEFT].lateral_line_thickness = value;
		UpdateItems(TMJ_LEFT);
		break;
	case RIGHT_INTERVAL:
		roi_info_[TMJ_RIGHT].lateral_line_interval = value;
		ClampShiftedLateralLine(TMJDirectionType::TMJ_RIGHT);
		UpdateItems(TMJ_RIGHT);
		break;
	case RIGHT_THICKNESS:
		roi_info_[TMJ_RIGHT].lateral_line_thickness = value;
		UpdateItems(TMJ_RIGHT);
		break;
	default:
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR,
			"TMJItem::SetLateralParam:");
		break;
	}
}
void TMJItem::GetLateralParam(const TMJLateralID& id, float* value) const
{
	switch (id)
	{
	case LEFT_INTERVAL:
		*value = roi_info_[TMJ_LEFT].lateral_line_interval;
		break;
	case LEFT_THICKNESS:
		*value = roi_info_[TMJ_LEFT].lateral_line_thickness;
		break;
	case RIGHT_INTERVAL:
		*value = roi_info_[TMJ_RIGHT].lateral_line_interval;
		break;
	case RIGHT_THICKNESS:
		*value = roi_info_[TMJ_RIGHT].lateral_line_thickness;
		break;
	default:
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR,
			"TMJItem::GetLateralParam:");
		break;
	}
}
void TMJItem::SetLateralLineCount(const TMJDirectionType& type, int count)
{
	roi_info_[type].lateral_line_count = count;
	if (IsValidTMJ(type)) UpdateItems(type);
}

void TMJItem::SetLateralSliceInterval(const TMJDirectionType& type,
	float interval)
{
	roi_info_[type].lateral_line_interval = interval;
	if (IsValidTMJ(type)) UpdateItems(type);
}

bool TMJItem::IsAvaliableAddPoint() const
{
	TMJDirectionType draw_type = this->GetDrawType();
	if (draw_type == TMJ_TYPE_UNKNOWN)
		return false;
	else
		return true;
}

bool TMJItem::IsStartEdit() const
{
	return (draw_line_.get() != nullptr && draw_line_->getCurveData().size() == 1)
		? true
		: false;
}

/*=============================================================================================
private functions
===============================================================================================*/
void TMJItem::UpdateItems(const TMJDirectionType& type)
{
	this->UpdateTMJRect(type);
	this->UpdateResizeArrow(type);
	this->UpdateLateralLine(type);
	this->UpdateFrontalLine(type);
	this->UpdateAngleItems(type);

	emit sigUpdated(type);
}
void TMJItem::UpdateTMJRect(const TMJDirectionType& type)
{
	if (!is_init)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJItem::InitVerticalLine: please call Initialize.");
	}

	QVector2D horizontal, vertical;
	GetFrontalVector(type, &horizontal);

	vertical = QVector2D(horizontal.y(), -horizontal.x());

	QPointF offset_vertical =
		VtoQPointF(horizontal * roi_info_[type].width * 0.5);
	QPointF offset_horizon = VtoQPointF(vertical * roi_info_[type].height * 0.5);

	QPolygonF poly;
	poly.push_back(roi_info_[type].center + offset_vertical + offset_horizon);
	poly.push_back(roi_info_[type].center + offset_vertical - offset_horizon);
	poly.push_back(roi_info_[type].center - offset_vertical - offset_horizon);
	poly.push_back(roi_info_[type].center - offset_vertical + offset_horizon);

	tmj_widget_[type].roi->setPolygon(poly);
	tmj_widget_[type].roi->setVisible(true);
}

void TMJItem::UpdateLateralLine(const TMJDirectionType& type)
{
	if (!is_init)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJItem::UpdateVerticalLine: please call Initialize.");
	}

	auto QVector2DtoQPointF = [](const QVector2D& v) -> QPointF
	{
		return QPointF(v.x(), v.y());
	};

	QVector2D horizontal_vector;
	GetFrontalVector(type, &horizontal_vector);

	QVector2D vertical_vector =
		QVector2D(horizontal_vector.y(), -horizontal_vector.x());

	float pen_thickness = std::max(roi_info_[type].lateral_line_thickness, 1.0f);
	QPen line_pen = QPen(ColorTmjItem::kLateralLineNormal, pen_thickness,
		Qt::SolidLine, Qt::FlatCap);
	line_pen.setCosmetic(true);
	tmj_widget_[type].lateral_line->set_pen(line_pen);

	roi_info_[type].line_id.clear();
	tmj_widget_[type].lateral_line->ClearLines();
	tmj_widget_[type].lateral_line->set_length(roi_info_[type].height);
	tmj_widget_[type].lateral_line->setVisible(
		roi_info_[type].is_lateral_visible_);
	int line_start = -roi_info_[type].lateral_line_count / 2;
	for (int k = 0; k < roi_info_[type].lateral_line_count; k++)
	{
		QVector2D stride_v_line =
			((float)line_start * roi_info_[type].lateral_line_interval +
				roi_info_[type].lateral_line_shifted) *
			horizontal_vector;

		QPointF v_line_pos =
			QVector2DtoQPointF(stride_v_line) + roi_info_[type].center;
		bool is_contain = tmj_widget_[type].roi->contains(v_line_pos);

		if (is_contain)
		{
			roi_info_[type].line_id.push_back(
				(QVector2D::dotProduct(stride_v_line, horizontal_vector)) +
				roi_info_[type].width * 0.5f);
			tmj_widget_[type].lateral_line->AddLine(v_line_pos, vertical_vector);
		}
		line_start++;
	}
	UpdateHighlightLateralLine(type);
}

void TMJItem::UpdateFrontalLine(const TMJDirectionType& type)
{
	if (!is_init)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJItem::UpdateFrontalLine: please call Initialize.");
	}

	QVector2D v1 =
		QVector2D(tmj_widget_[type].lateral_resize_arrow[0]->GetStart());
	QVector2D v2 = QVector2D(tmj_widget_[type].lateral_resize_arrow[0]->GetEnd());

	QVector2D lateral_vector;
	GetLateralVector(type, &lateral_vector);

	QPointF p1_frontal = VtoQPointF(
		(v1 + v2) * 0.5 + lateral_vector * roi_info_[type].frontal_line_shifted);

	v1 = QVector2D(tmj_widget_[type].lateral_resize_arrow[1]->GetStart());
	v2 = QVector2D(tmj_widget_[type].lateral_resize_arrow[1]->GetEnd());
	QPointF p2_frontal = VtoQPointF(
		(v1 + v2) * 0.5 + lateral_vector * roi_info_[type].frontal_line_shifted);
	if (type == TMJ_LEFT)
	{
		tmj_widget_[type].frontal_ellipse[1]->setPos(p1_frontal);
		tmj_widget_[type].frontal_ellipse[0]->setPos(p2_frontal);
	}
	else if (type == TMJ_RIGHT)
	{
		tmj_widget_[type].frontal_ellipse[0]->setPos(p1_frontal);
		tmj_widget_[type].frontal_ellipse[1]->setPos(p2_frontal);
	}
	else
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR,
			"TMJItem::UpdateFrontalLine:");
	}

	QVector2D vec = QVector2D(p2_frontal - p1_frontal);
	float line_length = vec.length();

	vec.normalize();
	QPointF center = (p1_frontal + p2_frontal) * 0.5;

	tmj_widget_[type].frontal_line->ClearLines();
	tmj_widget_[type].frontal_line->set_length(line_length);
	tmj_widget_[type].frontal_line->setVisible(
		roi_info_[type].is_frontal_visible_);
	tmj_widget_[type].frontal_line->AddLine(center, vec);

	tmj_widget_[type].frontal_ellipse[0]->setVisible(true);
	tmj_widget_[type].frontal_ellipse[1]->setVisible(true);
}

void TMJItem::UpdateAngleItems(const TMJDirectionType& type)
{
	if (!is_init)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJItem::UpdateAngleItems: please call Initialize.");
	}
	const auto& crit_points = criterion_line_->getCurveData();
	QVector2D dir_crit_line =
		QVector2D(crit_points.back() - crit_points.front()).normalized();

	float arc_length = angle_length_ * 0.4f;

	QVector2D h_dir_tmj_line;
	this->GetFrontalVector(type, &h_dir_tmj_line);
	QVector2D cross_dir_crit_line =
		QVector2D(dir_crit_line.y(), -dir_crit_line.x());
	cross_dir_crit_line.normalize();

	float degree =
		acos(QVector2D::dotProduct(h_dir_tmj_line, cross_dir_crit_line)) *
		180.0f / M_PI;

	QVector2D orig_vector(1.0f, 0.0f);
	float ori_degree =
		acos(QVector2D::dotProduct(orig_vector, cross_dir_crit_line)) * 180.0f /
		M_PI;

	degree = (h_dir_tmj_line.x() * cross_dir_crit_line.y() -
		h_dir_tmj_line.y() * cross_dir_crit_line.x() >
		0.0f)
		? degree
		: -degree;
	ori_degree = (orig_vector.x() * cross_dir_crit_line.y() -
		orig_vector.y() * cross_dir_crit_line.x() <
		0.0f)
		? ori_degree
		: -ori_degree;

	QPointF h_point, text_pos, angle_point;
	QPointF angle_ori_pos =
		crit_points.front() + VtoQPointF(dir_crit_line * angle_length_ * 0.5f);
	if (type == TMJ_RIGHT)
	{
		h_point = angle_ori_pos - VtoQPointF(cross_dir_crit_line * angle_length_);
		angle_point =
			angle_ori_pos - VtoQPointF(cross_dir_crit_line * angle_length_ * 0.5f);
		angle_widget_[type].text->setPlainText(
			QString("R : ") + QString::number(-degree, 'f', 1) + QString(" deg"));
		text_pos = h_point;

		QTransform text_transform;
		text_transform.rotate(-ori_degree);
		angle_widget_[type].text->setPos(text_pos);
		angle_widget_[type].text->setTransform(text_transform);

		ori_degree -= 180.0f;
	}
	else
	{
		h_point = angle_ori_pos + VtoQPointF(cross_dir_crit_line * angle_length_);
		angle_point =
			angle_ori_pos + VtoQPointF(cross_dir_crit_line * angle_length_ * 0.5f);
		angle_widget_[type].text->setPlainText(
			QString("L : ") + QString::number(degree, 'f', 1) + QString(" deg"));
		QPointF text_pos_offset =
			QPointF(angle_widget_[type].text->sceneBoundingRect().width(), 0.0f);
		text_pos = h_point - text_pos_offset;

		QTransform text_transform;
		text_transform.translate(text_pos_offset.x(), text_pos_offset.y());
		text_transform.rotate(-ori_degree);
		text_transform.translate(-text_pos_offset.x(), -text_pos_offset.y());
		angle_widget_[type].text->setPos(text_pos);
		angle_widget_[type].text->setTransform(text_transform);
	}

	QTransform transform;
	transform.translate(angle_ori_pos.x(), angle_ori_pos.y());
	transform.rotate(-degree);
	transform.translate(-angle_ori_pos.x(), -angle_ori_pos.y());
	QPointF arc_point = transform.map(angle_point);

	angle_widget_[type].arc_line->setLine(arc_point.x(), arc_point.y(),
		angle_ori_pos.x(), angle_ori_pos.y());
	angle_widget_[type].arc_line->setVisible(true);

	angle_widget_[type].h_line->setLine(h_point.x(), h_point.y(),
		angle_ori_pos.x(), angle_ori_pos.y());
	angle_widget_[type].h_line->setVisible(true);

	angle_widget_[type].arc->setRect(
		angle_ori_pos.x() - (double)(arc_length * 0.5f),
		angle_ori_pos.y() - (double)(arc_length * 0.5f), arc_length, arc_length);
	angle_widget_[type].arc->setStartAngle(ori_degree * 16);
	angle_widget_[type].arc->setSpanAngle(degree * 16);
	angle_widget_[type].arc->setVisible(true);
}

void TMJItem::UpdateResizeArrow(const TMJDirectionType& type)
{
	if (!is_init)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJItem::UpdateResizeArrow: please call Initialize.");
	}

	const QPolygonF& poly = tmj_widget_[type].roi->polygon();

	if (poly.size() != 4)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJItem::UpdateResizeArrow: check roi's polygon count.");
	}

	enum Dir { Vertical, Horizon, End };

	QPointF p1, p2, p3, p4;
	if (type == TMJ_LEFT)
	{
		p1 = poly[3];
		p2 = poly[2];
		p3 = poly[0];
		p4 = poly[1];
	}
	else if (type == TMJ_RIGHT)
	{
		p1 = poly[1];
		p2 = poly[0];
		p3 = poly[2];
		p4 = poly[3];
	}
	else
	{
	}

	tmj_widget_[type].lateral_resize_arrow[0]->SetStart(p1);
	tmj_widget_[type].lateral_resize_arrow[0]->SetEnd(p2);
	tmj_widget_[type].lateral_resize_arrow[1]->SetStart(p4);
	tmj_widget_[type].lateral_resize_arrow[1]->SetEnd(p3);

	tmj_widget_[type].frontal_resize_arrow[0]->SetStart(p1);
	tmj_widget_[type].frontal_resize_arrow[0]->SetEnd(p3);
	tmj_widget_[type].frontal_resize_arrow[1]->SetStart(p4);
	tmj_widget_[type].frontal_resize_arrow[1]->SetEnd(p2);
}

void TMJItem::CreateTMJwidget(const TMJDirectionType& type)
{
	this->CreateROIItem(type);
	this->CreateLateralLine(type);
	this->CreateFrontalEllipse(type);
	this->CreateFrontalLine(type);
	this->CreateResizeArrow(type);
}

void TMJItem::CreateAngleWidget(const TMJDirectionType& type)
{
	this->CreateAngleArc(type);
	this->CreateAngleArcLine(type);
	this->CreateAngleHorizonLine(type);
	this->CreateAngleText(type);
}

void TMJItem::CreateROIItem(const TMJDirectionType& type)
{
	QPen ROIPen(ColorTmjItem::kROIpen, kLineWidth, Qt::SolidLine);

	ROIPen.setCosmetic(true);

	tmj_widget_[type].roi.reset(new PolygonItem(this));
	tmj_widget_[type].roi->SetPen(ROIPen);
	tmj_widget_[type].roi->SetHighlighted(true);
	tmj_widget_[type].roi->setFlag(PolygonItem::ItemIsMovable, false);
	tmj_widget_[type].roi->setZValue(kZorderROI);
	tmj_widget_[type].roi->setVisible(false);

	connect(tmj_widget_[type].roi.get(), &PolygonItem::sigTranslateRect, this,
		&TMJItem::slotTranslateTmjROI);
}

void TMJItem::CreateLateralLine(const TMJDirectionType& type)
{
	QPen line_pen = QPen(ColorTmjItem::kLateralLineNormal, kLineWidth,
		Qt::SolidLine, Qt::FlatCap);
	line_pen.setCosmetic(true);

	tmj_widget_[type].lateral_line.reset(new LineListItem(this));
	tmj_widget_[type].lateral_line->setZValue(kZorderVLine);
	tmj_widget_[type].lateral_line->SetHighlight(true);
	tmj_widget_[type].lateral_line->SetMovable(false);
	tmj_widget_[type].lateral_line->set_pen(line_pen);
	tmj_widget_[type].lateral_line->setVisible(false);
	connect(tmj_widget_[type].lateral_line.get(),
		&LineListItem::sigTranslateLines, this,
		&TMJItem::slotTranslateTmjLines);
}

void TMJItem::CreateFrontalLine(const TMJDirectionType& type)
{
	QPen line_pen = QPen(ColorTmjItem::kFrontalLineNormal, kLineWidth,
		Qt::SolidLine, Qt::FlatCap);
	line_pen.setCosmetic(true);

	tmj_widget_[type].frontal_line.reset(new LineListItem(this));
	tmj_widget_[type].frontal_line->setZValue(kZorderHLine);
	tmj_widget_[type].frontal_line->SetHighlight(true);
	tmj_widget_[type].frontal_line->SetMovable(true);
	tmj_widget_[type].frontal_line->set_pen(line_pen);
	tmj_widget_[type].frontal_line->setVisible(false);
	connect(tmj_widget_[type].frontal_line.get(),
		&LineListItem::sigTranslateLines, this,
		&TMJItem::slotTranslateTmjLines);
}

void TMJItem::CreateFrontalEllipse(const TMJDirectionType& type)
{
	QPen ellipse_pen = QPen(ColorTmjItem::kFrontalLineNormal, kLineWidth,
		Qt::SolidLine, Qt::FlatCap);
	ellipse_pen.setCosmetic(true);

	for (int k = 0; k < 2; k++)
	{
		tmj_widget_[type].frontal_ellipse[k].reset(new CW3EllipseItem(this));
		tmj_widget_[type].frontal_ellipse[k]->setZValue(kZorderEllipse);
		tmj_widget_[type].frontal_ellipse[k]->setPen(ellipse_pen);
		tmj_widget_[type].frontal_ellipse[k]->setBrush(
			QBrush(ColorTmjItem::kFrontalEllBrush));
		tmj_widget_[type].frontal_ellipse[k]->SetFlagHighlight(true);
		tmj_widget_[type].frontal_ellipse[k]->SetFlagMovable(false);
		tmj_widget_[type].frontal_ellipse[k]->setVisible(false);

		connect(tmj_widget_[type].frontal_ellipse[k].get(),
			SIGNAL(sigTranslateEllipse(QPointF)), this,
			SLOT(slotTranslateTmjEll(QPointF)));
	}
}

void TMJItem::CreateResizeArrow(const TMJDirectionType& type)
{
	QPen line_pen = QPen(ColorTmjItem::kLateralLineNormal, kLineWidth,
		Qt::SolidLine, Qt::FlatCap);
	line_pen.setCosmetic(true);

	for (int k = 0; k < 2; k++)
	{
		tmj_widget_[type].lateral_resize_arrow[k].reset(new TmjArrowItem(this));
		tmj_widget_[type].lateral_resize_arrow[k]->SetPen(line_pen);
		tmj_widget_[type].lateral_resize_arrow[k]->SetBrush(
			QBrush(ColorTmjItem::kLateralLineNormal));
		tmj_widget_[type].lateral_resize_arrow[k]->setVisible(true);
		tmj_widget_[type].lateral_resize_arrow[k]->setZValue(kZorderArrow);

		connect(tmj_widget_[type].lateral_resize_arrow[k].get(),
			&TmjArrowItem::sigHover, this,
			&TMJItem::slotHoverTmjResizeVertical);

		tmj_widget_[type].frontal_resize_arrow[k].reset(new TmjArrowItem(this));
		tmj_widget_[type].frontal_resize_arrow[k]->SetPen(line_pen);
		tmj_widget_[type].frontal_resize_arrow[k]->SetBrush(
			QBrush(ColorTmjItem::kLateralLineNormal));
		tmj_widget_[type].frontal_resize_arrow[k]->setVisible(true);
		tmj_widget_[type].frontal_resize_arrow[k]->setZValue(kZorderArrow);

		connect(tmj_widget_[type].frontal_resize_arrow[k].get(),
			&TmjArrowItem::sigHover, this, &TMJItem::slotHoverTmjResizeHorizon);
	}
}

void TMJItem::CreateCriterionLine()
{
	QPen ell_pen(ColorTmjItem::kCriterionLine, kLineWidth, Qt::SolidLine);
	ell_pen.setCosmetic(true);

	QPen line_pen(ColorTmjItem::kCriterionLine, kLineWidth, Qt::DashDotLine);
	line_pen.setCosmetic(true);

	criterion_line_.reset(new CW3Curve(0, this));
	criterion_line_->setPenPath(line_pen);
	criterion_line_->setPenEllipse(ell_pen);
	criterion_line_->setBrushEllipse(QBrush(ColorTmjItem::kCriterionEllBrush));
	criterion_line_->setHighligtedEllipse(true);
	criterion_line_->setHighligtedPath(true);

	connect(criterion_line_.get(), &CW3Curve::sigTranslateEllipse, this,
		&TMJItem::slotTranslateCriterion);
	connect(criterion_line_.get(), &CW3Curve::sigTranslatePath, this,
		&TMJItem::slotTranslateCriterion);
}

void TMJItem::CreateDrawLine()
{
	QPen ell_pen(ColorTmjItem::kCriterionLine, kLineWidth, Qt::SolidLine);
	ell_pen.setCosmetic(true);

	QPen line_pen(ColorTmjItem::kCriterionLine, kLineWidth, Qt::DashDotLine);
	line_pen.setCosmetic(true);

	draw_line_.reset(new CW3Curve(0, this));
	draw_line_->setPenPath(line_pen);
	draw_line_->setPenEllipse(ell_pen);
	draw_line_->setBrushEllipse(QBrush(ColorTmjItem::kCriterionEllBrush));
	draw_line_->setHighligtedEllipse(true);
	draw_line_->setHighligtedPath(true);
}

void TMJItem::CreateAngleArc(const TMJDirectionType& type)
{
	QPen angle_pen[TMJ_TYPE_END];
	angle_pen[TMJ_LEFT].setColor(ColorTmjItem::kAngleArcLeft);
	angle_pen[TMJ_RIGHT].setColor(ColorTmjItem::kAngleArcRight);

	angle_pen[type].setWidthF(kLineWidth);
	angle_pen[type].setCosmetic(true);

	angle_widget_[type].arc.reset(new CW3ArcItem(this));
	angle_widget_[type].arc->setZValue(0.0f);
	angle_widget_[type].arc->setPen(angle_pen[type]);
	angle_widget_[type].arc->setVisible(false);
}

void TMJItem::CreateAngleArcLine(const TMJDirectionType& type)
{
	QPen angle_pen[TMJ_TYPE_END];
	angle_pen[TMJ_LEFT].setColor(ColorTmjItem::kAngleArcLeft);
	angle_pen[TMJ_RIGHT].setColor(ColorTmjItem::kAngleArcRight);

	QPen line_pen(ColorTmjItem::kCriterionLine, kLineWidth, Qt::DashDotLine);
	line_pen.setCosmetic(true);

	angle_pen[type].setWidthF(1.0f);
	angle_pen[type].setCosmetic(true);

	angle_widget_[type].arc_line.reset(new CW3LineItem(0, this));
	angle_widget_[type].arc_line->setZValue(0.0f);
	angle_widget_[type].arc_line->setPen(angle_pen[type]);
	angle_widget_[type].arc_line->setVisible(false);
}

void TMJItem::CreateAngleHorizonLine(const TMJDirectionType& type)
{
	QPen line_pen(ColorTmjItem::kCriterionLine, kLineWidth, Qt::DashDotLine);
	line_pen.setCosmetic(true);

	angle_widget_[type].h_line.reset(new CW3LineItem(0, this));
	angle_widget_[type].h_line->setZValue(0.0f);
	angle_widget_[type].h_line->setPen(line_pen);
	angle_widget_[type].h_line->setVisible(false);
}

void TMJItem::CreateAngleText(const TMJDirectionType& type)
{
	angle_widget_[type].text.reset(new CW3TextItem(this));
	angle_widget_[type].text->setAcceptHoverEvents(false);
	angle_widget_[type].text->setTextBold(true);
	angle_widget_[type].text->setZValue(0.0f);
	angle_widget_[type].text->setAntialiasing(false);
}

void TMJItem::DeleteTMJwidget(const TMJDirectionType& type)
{
	for (int k = 0; k < 2; k++)
	{
		tmj_widget_[type].frontal_ellipse[k].reset(nullptr);
		tmj_widget_[type].lateral_resize_arrow[k].reset(nullptr);
		tmj_widget_[type].frontal_resize_arrow[k].reset(nullptr);
	}
	tmj_widget_[type].frontal_line.reset(nullptr);
	tmj_widget_[type].lateral_line.reset(nullptr);
	tmj_widget_[type].roi.reset(nullptr);
}

void TMJItem::DeleteAngleWidget(const TMJDirectionType& type)
{
	angle_widget_[type].arc.reset(nullptr);
	angle_widget_[type].arc_line.reset(nullptr);
	angle_widget_[type].h_line.reset(nullptr);
	angle_widget_[type].text.reset(nullptr);
}
void TMJItem::DeleteDrawLine() { draw_line_.reset(nullptr); }
void TMJItem::GetFrontalVector(const TMJDirectionType& type,
	QVector2D* vec) const
{
	QPointF p1 = tmj_widget_[type].frontal_ellipse[0]->pos();
	QPointF p2 = tmj_widget_[type].frontal_ellipse[1]->pos();
	*vec = QVector2D(p1 - p2).normalized();
}

void TMJItem::GetLateralVector(const TMJDirectionType& type,
	QVector2D* vec) const
{
	QPointF p1 = tmj_widget_[type].frontal_ellipse[0]->pos();
	QPointF p2 = tmj_widget_[type].frontal_ellipse[1]->pos();
	QVector2D v1 = QVector2D(p1 - p2);
	*vec = QVector2D(v1.y(), -v1.x()).normalized();
}
bool TMJItem::TranslateTmjResize(const QPointF& trans)
{
	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		TMJDirectionType type = static_cast<TMJDirectionType>(i);
		if (!IsValidTMJ(type))
		{
			continue;
		}
		if (TranslateTmjResizeVertical(type, trans))
			return true;
		else if (TranslateTmjResizeHorizon(type, trans))
			return true;
	}

	return false;
}
bool TMJItem::TranslateTmjResizeVertical(const TMJDirectionType& type,
	const QPointF& trans)
{
	TmjArrowItem* curr_hover_arrow = nullptr;
	for (int k = 0; k < 2; k++)
	{
		if (tmj_widget_[type].lateral_resize_arrow[k]->IsVisible())
		{
			for (int i = 0; i < 2; i++)
			{
				if (tmj_widget_[type].frontal_resize_arrow[i]->IsHovered())
				{
					curr_hover_arrow = tmj_widget_[type].frontal_resize_arrow[i].get();
					break;
				}
			}
		}
	}

	if (!curr_hover_arrow) return false;

	QVector2D line_trans_vector;
	line_trans_vector =
		QVector2D(curr_hover_arrow->GetEnd() - curr_hover_arrow->GetStart());
	line_trans_vector =
		QVector2D(line_trans_vector.y(), -line_trans_vector.x()).normalized();
	float trans_scalar = QVector2D::dotProduct(QVector2D(trans.x(), trans.y()),
		line_trans_vector) *
		2.0f;
	roi_info_[type].height += trans_scalar;
	roi_info_[type].height = std::max(roi_info_[type].height, kAvailableHeight);
	UpdateItems(type);
	ClampShiftedFrontalLine(type);

	return true;
}

bool TMJItem::TranslateTmjResizeHorizon(const TMJDirectionType& type,
	const QPointF& trans)
{
	TmjArrowItem* curr_hover_arrow = nullptr;
	for (int k = 0; k < 2; k++)
	{
		if (tmj_widget_[type].frontal_resize_arrow[k]->IsVisible())
		{
			for (int i = 0; i < 2; i++)
			{
				if (tmj_widget_[type].lateral_resize_arrow[i]->IsHovered())
				{
					curr_hover_arrow = tmj_widget_[type].lateral_resize_arrow[i].get();
					break;
				}
			}
		}
	}

	if (!curr_hover_arrow) return false;

	QVector2D line_trans_vector;
	line_trans_vector =
		QVector2D(curr_hover_arrow->GetEnd() - curr_hover_arrow->GetStart());
	line_trans_vector =
		QVector2D(line_trans_vector.y(), -line_trans_vector.x()).normalized();

	float trans_scalar = QVector2D::dotProduct(QVector2D(trans.x(), trans.y()),
		-line_trans_vector) *
		2.0f;

	float new_width = roi_info_[type].width + trans_scalar;
	float avaliable_width = (roi_info_[type].lateral_line_count - 1) *
		roi_info_[type].lateral_line_interval;
	if (new_width < avaliable_width)
	{
		trans_scalar -= new_width - roi_info_[type].width;
	}

	roi_info_[type].width += trans_scalar;
	ClampShiftedLateralLine(type);

	UpdateItems(type);
	return true;
}

void TMJItem::GetAvailableLateralRange(const TMJDirectionType& type, float& min, float& max)
{
	int cnt_max, cnt_min;

	cnt_max = roi_info_[type].lateral_line_count / 2;
	cnt_min = roi_info_[type].lateral_line_count / 2;
	if (roi_info_[type].lateral_line_count % 2 == 0)
	{
		cnt_max -= 1;
	}

	max = roi_info_[type].width * 0.5f - static_cast<float>(std::max(cnt_max, 0)) * roi_info_[type].lateral_line_interval;
	min = -roi_info_[type].width * 0.5f + static_cast<float>(cnt_min) * roi_info_[type].lateral_line_interval;
}

void TMJItem::GetAvailableFrontalRange(const TMJDirectionType& type, float& min, float& max)
{
	min = -roi_info_[type].height * 0.5f;
	max = roi_info_[type].height * 0.5f;
}

bool TMJItem::IsShiftableLateralLine(const TMJDirectionType& type, const float shift)
{
	float available_shifted_min = 0.0f;
	float available_shifted_max = 0.0f;
	GetAvailableLateralRange(type, available_shifted_min, available_shifted_max);

	float current_shifted = roi_info_[type].lateral_line_shifted;
	if ((available_shifted_max == current_shifted && current_shifted + shift >= available_shifted_max) ||
		(available_shifted_min == current_shifted && current_shifted + shift <= available_shifted_min))
	{
		return false;
	}

	return true;
}

bool TMJItem::IsShiftableFrontalLine(const TMJDirectionType& type, const float shift)
{
	float available_shifted_min = 0.0f;
	float available_shifted_max = 0.0f;
	GetAvailableFrontalRange(type, available_shifted_min, available_shifted_max);

	float current_shifted = roi_info_[type].frontal_line_shifted;
	if ((available_shifted_max == current_shifted && current_shifted + shift >= available_shifted_max) ||
		(available_shifted_min == current_shifted && current_shifted + shift <= available_shifted_min))
	{
		return false;
	}

	return true;
}

void TMJItem::ClampShiftedLateralLine(const TMJDirectionType& type)
{
	int cnt_max, cnt_min;

	cnt_max = roi_info_[type].lateral_line_count / 2;
	cnt_min = roi_info_[type].lateral_line_count / 2;
	if (roi_info_[type].lateral_line_count % 2 == 0)
	{
		cnt_max -= 1;
	}

	float available_shifted_max =
		roi_info_[type].width * 0.5f -
		(float)std::max(cnt_max, 0) * roi_info_[type].lateral_line_interval;

	float available_shifted_min =
		-roi_info_[type].width * 0.5f +
		(float)(cnt_min)*roi_info_[type].lateral_line_interval;

	roi_info_[type].lateral_line_shifted =
		std::min(available_shifted_max, roi_info_[type].lateral_line_shifted);
	roi_info_[type].lateral_line_shifted =
		std::max(available_shifted_min, roi_info_[type].lateral_line_shifted);
}

void TMJItem::ClampShiftedFrontalLine(const TMJDirectionType& type)
{
	float available_shifted_max = roi_info_[type].height * 0.5f;
	float available_shifted_min = -roi_info_[type].height * 0.5f;

	roi_info_[type].frontal_line_shifted =
		std::min(available_shifted_max, roi_info_[type].frontal_line_shifted);
	roi_info_[type].frontal_line_shifted =
		std::max(available_shifted_min, roi_info_[type].frontal_line_shifted);
}

bool TMJItem::IsValidTMJ(const TMJDirectionType& type) const
{
	return (is_init && tmj_widget_[type].roi.get() != nullptr) ? true : false;
}

TMJDirectionType TMJItem::GetDrawType() const
{
	TMJDirectionType draw_type = TMJ_TYPE_UNKNOWN;
	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		if (draw_on_[i])
		{
			draw_type = (TMJDirectionType)i;
		}
	}
	return draw_type;
}

void TMJItem::UpdateHighlightLateralLine(const TMJDirectionType& type) const
{
	if (!IsValidTMJ(type))
	{
		return;
	}
	const auto& lines = tmj_widget_[type].lateral_line->lines();
	int id = roi_info_[type].lateral_highlight_index;

	for (auto& elem : lines)
	{
		tmj_widget_[type].lateral_line->SetColorSpecificLine(
			ColorTmjItem::kLateralLineNormal, elem.first);
	}

	tmj_widget_[type].lateral_line->SetColorSpecificLine(
		ColorTmjItem::kLineHighlight, id);
}

void TMJItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseMoveEvent(event);

	QPointF trans = event->scenePos() - event->lastScenePos();
	TranslateTmjResize(trans);
}

/*=============================================================================================
private slots
===============================================================================================*/
void TMJItem::slotTranslateTmjEll(const QPointF& trans)
{
	TMJDirectionType type = TMJ_TYPE_UNKNOWN;
	int ellipse_idx;
	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		for (int k = 0; k < 2; k++)
		{
			if (QObject::sender() ==
				(QObject*)tmj_widget_[i].frontal_ellipse[k].get())
			{
				type = (TMJDirectionType)i;
				ellipse_idx = k;
				break;
			}
		}
	}

	if (type == TMJ_TYPE_UNKNOWN)
	{
		return;
	}
	QPointF new_ell_pos =
		tmj_widget_[type].frontal_ellipse[ellipse_idx]->pos() + trans;

	float new_width =
		QVector2D(new_ell_pos -
			tmj_widget_[type].frontal_ellipse[!ellipse_idx]->pos())
		.length();
	float minimum_width = (float)(roi_info_[type].lateral_line_count - 1) *
		roi_info_[type].lateral_line_interval;
	if (new_width <= minimum_width)
	{
		new_ell_pos = tmj_widget_[type].frontal_ellipse[ellipse_idx]->pos() +
			VtoQPointF(QVector2D(trans).normalized() *
			(roi_info_[type].width - minimum_width));
	}

	tmj_widget_[type].frontal_ellipse[ellipse_idx]->setPos(new_ell_pos);

	QVector2D vertical_vector;
	this->GetLateralVector(type, &vertical_vector);

	QPointF line_center =
		(tmj_widget_[type].frontal_ellipse[0]->pos() +
			tmj_widget_[type].frontal_ellipse[1]->pos()) *
		0.5 -
		VtoQPointF(vertical_vector * roi_info_[type].frontal_line_shifted);

	roi_info_[type].center = line_center;

	float prev_width = roi_info_[type].width;
	roi_info_[type].width =
		QVector2D(tmj_widget_[type].frontal_ellipse[0]->pos() -
			tmj_widget_[type].frontal_ellipse[1]->pos())
		.length();

	if (ellipse_idx == 1)
		roi_info_[type].lateral_line_shifted -=
		(roi_info_[type].width - prev_width) * 0.5f;
	else
		roi_info_[type].lateral_line_shifted +=
		(roi_info_[type].width - prev_width) * 0.5f;

	ClampShiftedLateralLine(type);

	this->UpdateItems(type);
}

void TMJItem::slotTranslateTmjLines(const QPointF& trans)
{
	LineListItem* line_list = nullptr;
	std::vector<CW3EllipseItem*> ellipse;
	QVector2D line_trans_vector;

	QPolygonF poly;
	QVector2D pt_trans_min, pt_trans_max;
	float* line_shifted = nullptr;
	std::vector<float>* line_number = nullptr;
	TMJDirectionType type;
	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		if (tmj_widget_[i].roi) poly = tmj_widget_[i].roi->polygon();

		if (QObject::sender() == (QObject*)tmj_widget_[i].lateral_line.get())
		{
			GetFrontalVector((TMJDirectionType)i, &line_trans_vector);
			line_list = tmj_widget_[i].lateral_line.get();
			pt_trans_min = QVector2D((poly[0] + poly[1]) * 0.5);
			pt_trans_max = QVector2D((poly[2] + poly[3]) * 0.5);
			line_shifted = &roi_info_[i].lateral_line_shifted;
			line_number = &roi_info_[i].line_id;
		}
		else if (QObject::sender() ==
			(QObject*)tmj_widget_[i].frontal_line.get())
		{
			GetLateralVector((TMJDirectionType)i, &line_trans_vector);
			line_list = tmj_widget_[i].frontal_line.get();
			pt_trans_min = QVector2D((poly[0] + poly[3]) * 0.5);
			pt_trans_max = QVector2D((poly[1] + poly[2]) * 0.5);
			ellipse.push_back(tmj_widget_[i].frontal_ellipse[0].get());
			ellipse.push_back(tmj_widget_[i].frontal_ellipse[1].get());
			line_shifted = &roi_info_[i].frontal_line_shifted;
		}

		if (!line_list) continue;

		type = (TMJDirectionType)i;
		break;
	}

	if (!line_list) return;

	const auto& lines = line_list->lines();
	float trans_scalar =
		QVector2D::dotProduct(QVector2D(trans.x(), trans.y()), line_trans_vector);
	QVector2D trans_vector = trans_scalar * line_trans_vector;

	QTransform transform;
	transform.translate(trans_vector.x(), trans_vector.y());

	for (const auto& elem : lines)
	{
		QPointF pt_line_center = line_list->GetLineCenterPosition(elem.first);
		QPointF pt_transed = transform.map(pt_line_center);
		if (!poly.containsPoint(pt_transed, Qt::WindingFill))
		{
			QVector2D v1 = QVector2D(pt_trans_min - QVector2D(pt_line_center));
			QVector2D v2 = QVector2D(pt_trans_max - QVector2D(pt_line_center));

			transform.reset();
			if (v1.length() < v2.length())
			{
				trans_scalar = QVector2D::dotProduct(v1, line_trans_vector);
				trans_vector = trans_scalar * line_trans_vector;
			}
			else
			{
				trans_scalar = QVector2D::dotProduct(v2, line_trans_vector);
				trans_vector = trans_scalar * line_trans_vector;
			}
			transform.translate(trans_vector.x(), trans_vector.y());
		}
	}

	*line_shifted += trans_scalar;
	if (line_number)
	{
		for (auto& elem : *line_number)
		{
			elem += trans_scalar;
		}
	}

	for (const auto& elem : lines)
	{
		line_list->TransformLine(elem.first, transform);
	}

	for (auto& elem : ellipse)
	{
		elem->setPos(transform.map(elem->pos()));
	}

	emit sigUpdated(type);
}

void TMJItem::slotTranslateTmjROI(const QPointF& trans)
{
	bool result = TranslateTmjResize(trans);
	if (result) return;

	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		if (QObject::sender() == (QObject*)tmj_widget_[i].roi.get())
		{
			roi_info_[i].center += trans;
			UpdateItems((TMJDirectionType)i);
			return;
		}
	}
}

void TMJItem::slotTranslateCriterion()
{
	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		TMJDirectionType type = (TMJDirectionType)i;
		if (IsValidTMJ(type))
			this->UpdateAngleItems(type);
	}
}

void TMJItem::slotHoverTmjResizeVertical(bool is_hover)
{
	TMJDirectionType type = TMJ_TYPE_UNKNOWN;

	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		if (QObject::sender() ==
			(QObject*)tmj_widget_[i].lateral_resize_arrow[0].get() ||
			QObject::sender() ==
			(QObject*)tmj_widget_[i].lateral_resize_arrow[1].get())
		{
			type = (TMJDirectionType)i;
			break;
		}
	}

	if (type == TMJ_TYPE_UNKNOWN) return;

	tmj_widget_[type].frontal_resize_arrow[0]->SetVisible(is_hover);
	tmj_widget_[type].frontal_resize_arrow[1]->SetVisible(is_hover);
}

void TMJItem::slotHoverTmjResizeHorizon(bool is_hover)
{
	TMJDirectionType type = TMJ_TYPE_UNKNOWN;

	for (int i = 0; i < TMJ_TYPE_END; i++)
	{
		if (QObject::sender() ==
			(QObject*)tmj_widget_[i].frontal_resize_arrow[0].get() ||
			QObject::sender() ==
			(QObject*)tmj_widget_[i].frontal_resize_arrow[1].get())
		{
			type = (TMJDirectionType)i;
			break;
		}
	}
	if (type == TMJ_TYPE_UNKNOWN) return;

	tmj_widget_[type].lateral_resize_arrow[0]->SetVisible(is_hover);
	tmj_widget_[type].lateral_resize_arrow[1]->SetVisible(is_hover);
}
