#pragma once
#include <string>

namespace project {
namespace group {
// resource
const std::string kResource("/Resource");
const std::string kResMeasure(kResource + "/Measure");
const std::string kResMeasureViewInfo(kResMeasure + "/ViewInfo");
const std::string kResMeasureDatas(kResMeasure + "/Data");
const std::string kMeasure("Measure");
const std::string kMeasurePoints("Points");
const std::string kMeasure3D("/Measure3D");

const std::string kResPanoEngine(kResource + "/Panorama");
const std::string kResPanoMaxilla(kResPanoEngine + "/PanoMaxilla");
const std::string kResPanoMandible(kResPanoEngine + "/PanoMandible");
const std::string kResCrossSection(kResPanoEngine + "/CS");
const std::string kResSagittal(kResPanoEngine + "/Sagittal");
const std::string kResNerve(kResPanoEngine + "/Nerve");
const std::string kResImplant(kResPanoEngine + "/Implant");
const std::string kNerveParams(kResPanoEngine + "/NerveParams");
const std::string kImplantData(kResImplant + "/implant_data");

const std::string kResMainVol(kResource + "/MainVol");
const std::string kMainVolData(kResMainVol + "/Data");
const std::string kMainVolDCMHeader(kResMainVol + "/DCMHeader");
const std::string kResSecondVol(kResource + "/SecVol");
const std::string kSecondVolData(kResSecondVol + "/Data");
const std::string kSecondVolDCMHeader(kResSecondVol + "/DCMHeader");

const std::string kResFace(kResource + "/Face");
const std::string kResAirway(kResource + "/Airway");

const std::string kMPREngine(kResource + "/MPREngine");
const std::string kCenterInVolume(kMPREngine + "/CenterInVolume");

// tab
const std::string kTab("/Tab");
const std::string kTabFile(kTab + "/File");

// mpr tab
const std::string kTabMPR(kTab + "/MPR");
const std::string kMPRTransformStatus(kTabMPR + "/TransformStatus");
const std::string kViewMPRAxial(kTabMPR + "/Axial");
const std::string kViewMPRSagittal(kTabMPR + "/Sagittal");
const std::string kViewMPRCoronal(kTabMPR + "/Coronal");
const std::string kViewMPR3D(kTabMPR + "/3D");
const std::string kViewMPRZoom3D(kTabMPR + "/Zoom3D");

// pano tab
const std::string kTabPanorama(kTab + "/Pano");
const std::string kViewPanoArch(kTabPanorama + "/PanoArch");
const std::string kViewPanoPano(kTabPanorama + "/PanoPano");
const std::string kViewPanoCS(kTabPanorama + "/PanoCS");

// implant tab
const std::string kTabImplant(kTab + "/Implant");
const std::string kViewImpArch(kTabImplant + "/ImpArch");
const std::string kViewImpSagittal(kTabImplant + "/ImpSagittal");
const std::string kViewImpPano(kTabImplant + "/ImpPano");
const std::string kViewImpCS(kTabImplant + "/ImpCS");
const std::string kViewImp3D(kTabImplant + "/Imp3D");

// tmj tab
const std::string kTabTMJ(kTab + "/TMJ");
const std::string kViewTMJAxial(kTabTMJ + "/Axial");
const std::string kViewTMJFrontalLeft(kTabTMJ + "/FrontalLeft");
const std::string kViewTMJFrontalRight(kTabTMJ + "/FrontalRight");
const std::string kViewTMJLateralLeft(kTabTMJ + "/LateralLeft");
const std::string kViewTMJLateralRight(kTabTMJ + "/LateralRight");
const std::string kViewTMJ3DLeft(kTabTMJ + "/3DLeft");
const std::string kViewTMJ3DRight(kTabTMJ + "/3DRight");
const std::string kTMJCutPoints(kTabTMJ + "/CutPoints");
const std::string kTMJCutPointsLeft(kTMJCutPoints + "/Left");
const std::string kTMJCutPointsRight(kTMJCutPoints + "/Right");

// ceph tab
const std::string kTabCeph(kTab + "/Ceph");
const std::string kViewCeph(kTabCeph + "/Ceph");
const std::string kSurgeryCutItems(kViewCeph + "/CutItems");
const std::string kSurgeryLandmarks(kViewCeph + "/Landmarks");
const std::string kSurgeryBar(kTabCeph + "/SurgeryBar");
const std::string kSurgeryBtnStatusText(kSurgeryBar + "/BtnStatusText");

// face tab
const std::string kTabFace(kTab + "/Face");
const std::string kViewFaceMesh(kTabFace + "/Mesh");
const std::string kViewFacePhoto(kTabFace + "/Photo");
const std::string kViewFaceBefore(kTabFace + "/Before");

// si tab
const std::string kTabSI(kTab + "/SI");
const std::string kViewSIAxial(kTabSI + "/Axial");
const std::string kViewSISagittal(kTabSI + "/Sagittal");
const std::string kViewSICoronal(kTabSI + "/Coronal");
const std::string kViewSIVR(kTabSI + "/VR");

// endo tab
const std::string kTabEndo(kTab + "/Endo");
const std::string kViewEndo(kTabEndo + "/Endo");
const std::string kViewEndoModify(kTabEndo + "/Modify");
const std::string kViewEndoSlice(kTabEndo + "/Slice");
const std::string kViewEndoSagittal(kTabEndo + "/Sagittal");
const std::string kEndoPath(kViewEndoSagittal + "/EndoPath");

// vtosto
const std::string kVTOSTO("/VTOSTO");
}  // end of namespace group

// group 으로 접근해야 함.
namespace ds {  // dataset
// general
const std::string kVersionInfo("VersionInfo");
const std::string kIsTabInit("is_tab_init");

// file
const std::string kVolInfo("vol_info");
const std::string kVolHistogram("histogram");

// mpr
const std::string kMPRRegistrationParams("mpr_reg_params");
const std::string kMPRThickness("mpr_thickness");
const std::string kMPRInterval("mpr_interval");
const std::string kMPRSecondTransform("second_transform");
const std::string kMPRTransformMeasureID("transform_measure_id");
const std::string kMPRTransformStatusCnt("transform_status_cnt");
const std::string kMPRTransformAxial("transform_axial");
const std::string kMPRTransformSagittal("transform_sagittal");
const std::string kMPRTransformCoronal("transform_coronal");
const std::string kMPRTransformRotCenter("transform_rot_center");
const std::string kMPRTransformAxialAngle("transform_axial_angle");
const std::string kMPRTransformSagittalAngle("transform_sagittal_angle");
const std::string kMPRTransformCoronalAngle("transform_coronal_angle");
const std::string kMPRCrossControllerCenterPos("cross_controller_center_pos");
const std::string kMPRCrossControllerCenterPosDelta("cross_controller_center_pos_delta");
const std::string kMPRCrossControllerRotateAngle("cross_controller_rotate_angle");

// measure
const std::string kMeasureCount("measure_count");
const std::string kMeasureInfo("measure_info");
const std::string kMeasureNoteText("measure_note_text");
const std::string kMeasureMemoText("measure_memo_text");
const std::string kViewParams("view_params");
const std::string kViewParamsCount("view_params_count");
const std::string kMeasureCounterpartMPR("counterpart_mpr");
const std::string kMeasureCounterpartPano("counterpart_pano");

// mpr engine
const std::string kMPRCenterInVolumeOrg("mpr_center_in_volume_org");
const std::string kMPRCenterInVolume("mpr_center_in_volume");
const std::string kSICenterInVolume("si_center_in_volume");

// pano engine
const std::string kPanoROI("pano_roi");
const std::string kPanoCurrArchType("arch_type");
const std::string kPanoReoriMatMandible("reori_mat_mandible");
const std::string kPanoReoriMatMaxilla("reori_mat_maxilla");
const std::string kNerveTotalCount("nerve_count");
const std::string kNerveID("nerve_id");
const std::string kNerveCtrlPoints("nerve_ctrl_points");
const std::string kNerveSplinePoints("nerve_spline_points");
const std::string kImplant3DMVP("implant_3d_mvp");
const std::string kCSShiftedValue("cs_shifted_value");
const std::string kPanoCtrlPoints("pano_ctrl_points");
const std::string kPanoShiftedValue("pano_shifted_value");
const std::string kPanoOrientMaxilla("pano_orient_maxillar");
const std::string kPanoOrientMandible("pano_orient_mandible");

// pano, implant
const std::string kCSInterval("cs_interval");
const std::string kCSAngle("cs_angle");
const std::string kCSThickness("cs_thickness");
const std::string kArchRange("arch_range");
const std::string kArchThickness("arch_thickness");

// implant
const std::string kImplantCount("implant_count");
const std::string kImplantMemo("memo");
const std::string kSelectedImplantID("selected_implant_id");
const std::string kImplantID("implant_id");
const std::string kImplantPlatformDiameter("platform_diameter");
const std::string kImplantCustomApicalDiameter("custom_apical_diameter");
const std::string kImplantDiameter("implant_diameter");
const std::string kImplantTotalLength("total_length");
const std::string kImplantLength("implant_length");
const std::string kImplantSubCategory("implant_sub_category");
const std::string kImplantPath("implant_path");
const std::string kImplantManufacturer("implant_manufacturer");
const std::string kImplantProduct("implant_product");
const std::string kImplantVisible("implant_visible");
const std::string kImplantPosInVol("implant_pos_in_vol");
const std::string kImplantRotInVol("implant_rot_in_vol");
const std::string kImplantTransInVol("implant_trans_in_vol");
//20250123 LIN 
const std::string kImplantabsolutePath("implant_absolute_path");

// tmj
const std::string kTMJRect("tmj_rect");
const std::string kTMJLateralParams("lateral_params");
const std::string kTMJMode("tmj_mode");
const std::string kTMJMemo("tmj_memo");
const std::string kTMJROI("tmj_roi");
const std::string kTMJReoriMat("reori_mat");
const std::string kTMJReorientAngle("reori_angle");
const std::string kTMJLateralUpVecLeft("lateral_up_vec_left");
const std::string kTMJLateralUpVecRight("lateral_up_vec_right");
const std::string kTMJRectCenterLeft("rect_center_left");
const std::string kTMJRectCenterRight("rect_center_right");
const std::string kTMJCutPolygon("cut_polygon");
const std::string kTMJCutPolygonCount("cut_polygon_cnt");

// view
const std::string k3DRotMatrix("rotate_3d");
const std::string kVolRange("vol_range");
const std::string kViewScale("view_scale");
const std::string kViewPlaneUpVector("plane_up_vector");
const std::string kViewPlaneBackVector("plane_back_vector");
const std::string kViewPlaneCenter("plane_center");
const std::string kViewPlaneDistanceFromVolumeCenter("plane_distance_from_volume_center");
const std::string kViewPlaneAvailableDepth("plane_available_depth");

// measure 3D
const std::string kMeasure3DCount("measure_3d_count");
const std::string kMeasure3DIndex("measure_3d_index");
const std::string kMeasure3DPoint("measure_3d_point");

// vtosto
const std::string kIsoValue("iso_value");
const std::string kHeadPoints("head_points");
const std::string kHeadTriIndices("head_tri_indices");
const std::string kFacePoints("face_points");
const std::string kFaceIndicies("face_indicies");
const std::string kFacePointsAfter("face_points_after");
const std::string kFaceTexCoords("face_tex_coords");
const std::string kModelPoints("model_points");
const std::string kModelTetraIndices("model_tetra_indices");
const std::string kModelTriIndices("model_tri_indices");
const std::string kModelTetraMoveResult("model_tetra_move_result");
const std::string kModelPhotoToSurface("model_photo_to_surface");
const std::string kVTOSTOFlags("vtosto_flags");

// ceph
const std::string kSurgeryCutOn("surgery_cut_on");
const std::string kSurgeryAdjust("surgery_adjust");
const std::string kSurgeryMove("surgery_move");
const std::string kSurgeryCutItemCount("surgery_cut_item_count");
const std::string kSrugeryCutPoints("surgery_cut_points");
const std::string kSrugeryCutTransMat("surgery_cut_trans");
const std::string kSrugeryCutRotMat("surgery_cut_rot");
const std::string kSrugeryCutScaleMat("surgery_cut_scale");
const std::string kSrugeryCutArcballMat("surgery_cut_arcball");
const std::string kSrugeryCutReoriMat("surgery_cut_reori");
const std::string kSurgeryLandmarkCount("surgery_landmark_count");
const std::string kSurgeryLandmarkName("surgery_landmark_name");
const std::string kSurgeryLandmarkPoint("surgery_landmark_point");
const std::string kSurgeryParams("params");
const std::string kSurgeryParamsPrev("params_prev");
const std::string kSurgeryOutterEdit("outer_edit");

// face
const std::string kTRDPath("trd_path");
const std::string kTRDPoints("trd_points");
const std::string kTRDNormals("trd_normals");
const std::string kTRDIndices("trd_indices");
const std::string kTRDTexCoords("trd_tex_coords");
const std::string kTRDTexImage("trd_tex_image");
const std::string kTRDTexWidth("trd_tex_w");
const std::string kTRDTexHeight("trd_tex_h");

// si
const std::string kSISecondToFirst("second_to_first");
const std::string kSISecondTransform("second_transform");
const std::string kSISecondRotate("second_rotate");
const std::string kSISecondTranslate("second_translate");
const std::string kSISecondRotateForMPR("second_rotate_for_mpr");
const std::string kSISecondTranslateForMPR("second_translate_for_mpr");

// endo
const std::string kEndoCurrPathNum("curr_path_num");
const std::string kAirwayData("airway_data");
const std::string kAirwaySize("airway_size");

}  // namespace ds

namespace ds_member {  // dataset member
const std::string kRangeStart("range_start");
const std::string kRangeEnd("range_end");
const std::string kAreaX("area_x");
const std::string kAreaY("area_y");
const std::string kTabType("tab_type");
const std::string kViewTypeID("view_type_id");
const std::string kCenterX("center_x");
const std::string kCenterY("center_y");
const std::string kWidth("width");
const std::string kHeight("height");
const std::string kDepth("depth");
const std::string kMin("min");
const std::string kMax("max");
const std::string kPixelSpacing("pixel_spacing");
const std::string kSliceSpacing("slice_spacing");
const std::string kSceneToGL("scene_to_gl");
const std::string kThickness("thickness");
const std::string kWindowCenter("window_center");
const std::string kWindowWidth("window_width");
const std::string kSlope("slope");
const std::string kIntercept("intercept");
const std::string kHistoSize("histo_size");
const std::string kSliceLocMaxilla("slice_loc_maxilla");
const std::string kSliceLocTeeth("slice_loc_teeth");
const std::string kSliceLocChin("slice_loc_chin");
const std::string kSliceLocNose("slice_loc_nose");
const std::string kThresholdAir("threshold_air");
const std::string kThresholdTissue("threshold_tissue");
const std::string kThresholdBone("threshold_bone");
const std::string kSecondToFirst("second_to_first");
const std::string kMeasureID("measure_id");
const std::string kMeasureType("measure_type");
const std::string kMeasurePointCount("measure_point_count");
const std::string kMeasureNoteID("measure_note_id");
const std::string kMeasureProfileID("measure_profile_id");
const std::string kVectorUp("up");
const std::string kVectorCenter("center");
const std::string kVectorBack("back");
const std::string kX("x");
const std::string kY("y");
const std::string kZ("z");
const std::string kAngle("angle");
const std::string kSliceGap("slice_gap");
const std::string kNormal("normal");
const std::string kV1("v1");
const std::string kV2("v2");
const std::string kV3("v3");
const std::string kColor("color");
const std::string kColorValue("color_value");
const std::string kCntAttributes("cnt_attributes");
const std::string kVisible("visible");
const std::string kDiameter("diameter");
const std::string kRadius("radius");
const std::string kID("id");
const std::string kScale("scale");
// vtosto flags
const std::string kIsSetIsoValue("is_set_iso_value");
const std::string kIsGenerateHead("is_generate_head");
const std::string kIsMakeTetra("is_make_tetra");
const std::string kIsFixedIsoValueInSurgery("is_fiexed_iso_value_in_surgery");
const std::string kIsLandmark("is_landmark");
const std::string kIsCutFace("is_cut_face");
const std::string kIsDoMapping("is_do_mapping");
const std::string kIsCalcDisp("is_calc_disp");
const std::string kIsMakeSurf("is_make_surf");
const std::string kIsMakeField("is_make_field");
const std::string kIsLoadTRD("is_load_trd");
const std::string kSurgeryOnMaxilla("surgery_on_maxilla");
const std::string kSurgeryOnMandible("surgery_on_mandible");
const std::string kSurgeryOnChin("surgery_on_chin");
// tmj params
const std::string kLeftWidth("left_width");
const std::string kLeftHeight("left_height");
const std::string kLeftInterval("left_interval");
const std::string kLeftThickness("left_thickness");
const std::string kRightWidth("right_width");
const std::string kRightHeight("right_height");
const std::string kRightInterval("right_interval");
const std::string kRightThickness("right_thickness");
}  // namespace ds_member
}  // end of namespace project
