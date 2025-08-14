#include "will3d_id_parser.h"
#include "W3Enum.h"
#include "define_measure.h"
#include "define_view.h"
#include "language_pack.h"

using namespace lang;

unsigned int Will3DIDParser::measure_id_ = 0;
unsigned int Will3DIDParser::measure_note_id_ = 0;
unsigned int Will3DIDParser::measure_profile_id_ = 0;

Will3DIDParser::Will3DIDParser() {}

Will3DIDParser::~Will3DIDParser() {}

void Will3DIDParser::ResetMeasureIDs()
{
	measure_id_ = 0;
	measure_note_id_ = 0;
	measure_profile_id_ = 0;
}

unsigned int Will3DIDParser::GetMeasureID() { return ++measure_id_; }

unsigned int Will3DIDParser::GetMeasureProfileID()
{
	return ++measure_profile_id_;
}

QString Will3DIDParser::GetMeasureTypeText(
	const common::measure::MeasureType& type)
{
	switch (type)
	{
	case common::measure::MeasureType::LENGTH_LINE:
		return LanguagePack::txt_ruler();
	case common::measure::MeasureType::LENGTH_TAPELINE:
		return LanguagePack::txt_tape() + " " + LanguagePack::txt_line();
	case common::measure::MeasureType::LENGTH_CURVE:
		return LanguagePack::txt_tape() + " " + LanguagePack::txt_curve();
	case common::measure::MeasureType::ANGLE_THREEPOINT:
		return LanguagePack::txt_angle();
	case common::measure::MeasureType::PROFILE:
		return LanguagePack::txt_profile();
	case common::measure::MeasureType::AREA_LINE:
		return LanguagePack::txt_area();
	case common::measure::MeasureType::ROI_RECT:
		return LanguagePack::txt_roi();
	case common::measure::MeasureType::DRAW_FREEDRAW:
		return LanguagePack::txt_free_draw();
	case common::measure::MeasureType::NOTE:
		return LanguagePack::txt_note();
	case common::measure::MeasureType::DRAW_ARROW:
		return LanguagePack::txt_arrow();
	case common::measure::MeasureType::DRAW_CIRCLE:
		return LanguagePack::txt_circle();
	case common::measure::MeasureType::DRAW_RECT:
		return LanguagePack::txt_rectangle();
	case common::measure::MeasureType::DRAW_LINE:
		return LanguagePack::txt_line();
	}
	return QString();
}

QString Will3DIDParser::GetMeasurePositionText(
	const common::ViewTypeID& view_type)
{
	switch (view_type)
	{
	case common::ViewTypeID::MPR_AXIAL:
		return LanguagePack::txt_mpr() + "-" + LanguagePack::txt_axial();
	case common::ViewTypeID::MPR_SAGITTAL:
		return LanguagePack::txt_mpr() + "-" + LanguagePack::txt_sagittal();
	case common::ViewTypeID::MPR_CORONAL:
		return LanguagePack::txt_mpr() + "-" + LanguagePack::txt_coronal();
	case common::ViewTypeID::MPR_3D:
		return LanguagePack::txt_mpr() + "-" + LanguagePack::txt_3d();
	case common::ViewTypeID::MPR_ZOOM3D:
		return LanguagePack::txt_mpr() + "-" + LanguagePack::txt_3d_zoom();
	case common::ViewTypeID::LIGHTBOX:
		return LanguagePack::txt_lightbox();
	case common::ViewTypeID::PANO:
		return LanguagePack::txt_panorama();
	case common::ViewTypeID::PANO_ARCH:
		return LanguagePack::txt_panorama() + "-" + LanguagePack::txt_axial();
	case common::ViewTypeID::PANO_ORIENTATION:
		return LanguagePack::txt_panorama() + "-" +
			LanguagePack::txt_orientation();
	case common::ViewTypeID::CROSS_SECTION:
		return LanguagePack::txt_cross_section();
	case common::ViewTypeID::IMPLANT_SAGITTAL:
		return LanguagePack::txt_implant() + "-" + LanguagePack::txt_sagittal();
	case common::ViewTypeID::IMPLANT_3D:
		return LanguagePack::txt_implant() + "-" + LanguagePack::txt_3d();
	case common::ViewTypeID::IMPLANT_BONEDENSITY:
		return LanguagePack::txt_bone_density();
	case common::ViewTypeID::ENDO:
		return LanguagePack::txt_endoscopy();
	case common::ViewTypeID::ENDO_MODIFY:
		return LanguagePack::txt_endoscopy() + "-" + LanguagePack::txt_modify();
	case common::ViewTypeID::ENDO_SAGITTAL:
		return LanguagePack::txt_endoscopy() + "-" + LanguagePack::txt_sagittal();
	case common::ViewTypeID::ENDO_SLICE:
		return LanguagePack::txt_endoscopy_slice();
	case common::ViewTypeID::TMJ_FRONTAL_LEFT:
	case common::ViewTypeID::TMJ_FRONTAL_RIGHT:
		return LanguagePack::txt_tmj() + "-" + LanguagePack::txt_front();
	case common::ViewTypeID::TMJ_LATERAL_LEFT:
	case common::ViewTypeID::TMJ_LATERAL_RIGHT:
		return LanguagePack::txt_tmj() + "-" + LanguagePack::txt_lateral();
	case common::ViewTypeID::TMJ_ARCH:
		return LanguagePack::txt_tmj() + "-" + LanguagePack::txt_slice();
	case common::ViewTypeID::SUPERIMPOSITION:
		return LanguagePack::txt_superimposition() + "-" + LanguagePack::txt_3d();
	case common::ViewTypeID::CEPH:
	case common::ViewTypeID::FACE_AFTER:
		return LanguagePack::txt_3d_ceph() + "/" + LanguagePack::txt_after();
	case common::ViewTypeID::FACE_BEFORE:
		return LanguagePack::txt_3d_ceph() + "/" + LanguagePack::txt_before();
	case common::ViewTypeID::FACE_PHOTO:
		return LanguagePack::txt_photo();
	case common::ViewTypeID::FACE_SURFACE:
		return LanguagePack::txt_3d_surface();
	}
	return QString();
}

unsigned int Will3DIDParser::GetMeasureNoteID() { return ++measure_note_id_; }
