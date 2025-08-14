#pragma once
/*=========================================================================

File:		Enumeration Type Definition.
Language:	C++11
Library:	Standard C++ Library

=========================================================================*/
namespace common {
	enum class ReconTypeID { RECON_UNKNOWN, MPR, MIP, VR, X_RAY };
}
enum TabType {
  TAB_UNKNOWN = -1,
#ifndef WILL3D_VIEWER
  TAB_FILE,
#endif
  TAB_MPR,
  TAB_PANORAMA,
//20250210 LIN
//#ifndef WILL3D_VIEWER
  TAB_IMPLANT,
//#endif
  TAB_TMJ,
#ifndef WILL3D_LIGHT
  TAB_3DCEPH,
  TAB_FACESIM,
  TAB_SI,
  TAB_ENDO,
#endif
  TAB_REPORT,
  TAB_END
};

enum TMJDirectionType {
  TMJ_TYPE_UNKNOWN = -1,
  TMJ_RIGHT,
  TMJ_LEFT,
  TMJ_TYPE_END
};

enum class CommonToolTypeFile 
{ 
	SAVE, 
	CAPTURE, 
	PRINT, 
	CDEXPORT,
	PACS //210430 LTK
};
/* Line Type ID */
enum UILineType { HORIZONTAL, VERTICAL, LINE_END };

enum class VRCutTool { POLYGON, FREEDRAW };

/*
Visibility tool box's object visibility id
*/
enum VisibleID {
  NERVE = 0,
  IMPLANT,
#ifndef WILL3D_LIGHT
  SECONDVOLUME,
  AIRWAY,
  FACEPHOTO,
#endif
  VISIBLE_END
};

enum SIVisibleID { MAIN = 0, SECOND, BOTH, SI_VISIBLE_END };
#ifndef WILL3D_LIGHT
enum MPRTaskID { ZOOM3D = 0, OBLIQUE, CUT3D, STLEXPORT, DRAW_ARCH, MPR_TASK_END };
#else
enum MPRTaskID { ZOOM3D = 0, OBLIQUE, STLEXPORT, DRAW_ARCH, MPR_TASK_END };
#endif
enum PanoTaskID { AUTO = 0, MANUAL, PANO_TASK_END };

enum CephTaskID {
  COORDSYS = 0,
  TRACING,
  SELECT_METHOD,
  SURGERY,
  SHOW_SKIN,
  CEPH_TASK_END
};

enum FaceTaskID { GENERATE = 0, LOAD, MAPPING, CLEAR, COMPARE, FACE_TASK_END };
enum SITaskID { LOAD_NEW = 0, AUTO_REG, SI_TASK_END };
enum ReorientViewID { ORIENT_A = 0, ORIENT_R, ORIENT_I, REORI_VIEW_END };

enum EndoPathID { PATH1 = 0, PATH2, PATH3, PATH4, PATH5, ENDO_PATH_END };

enum EndoPlayerID {
  START_POS = 0,
  STOP,
  PLAY,
  END_POS,
  PREV,
  NEXT,
  PLAYER_END
};

enum EndoPlayerParamID { PLAY_SPEED = 0, PLAY_INTERVAL, PLAYER_PARAM_END };
enum EndoCameraDir { FORWARD = 0, BACKWARD, CAM_FIXED, ENDO_CAM_END };

enum TMJRectID { LEFT_W = 0, LEFT_H, RIGHT_W, RIGHT_H, TMJ_RECT_END };

enum class TmjLayoutType {
  DEFAULT_2D,
  LATERAL_MAIN_2D,
  FRONTAL_MAIN_2D,
  DEFAULT_3D
};

enum TMJLateralID {
  LEFT_INTERVAL = 0,
  LEFT_THICKNESS,
  RIGHT_INTERVAL,
  RIGHT_THICKNESS,
  TMJ_LATERAL_END
};

/* MPR View ID */
enum MPRViewType { MPR_UNKNOWN = -1, AXIAL, SAGITTAL, CORONAL, MPR_END };

enum class LightboxViewType { AXIAL, SAGITTAL, CORONAL, VIEW_TYPE_UNKNOWN };

/* clipping MPR-plane specifiers. */
enum class MPRClipID { UNKNOWN, AXIAL, SAGITTAL, CORONAL, MPROVERLAY };

namespace surgery {
enum CutTypeID { MAXILLA = 0, MANDIBLE, CHIN, CUT_TYPE_END };

enum TranslateID { RL = 0, AP, SI, TRANS_END };

enum RotateID { SAG = 0, COR, AXI, ROT_END };
}  // namespace surgery

enum class DCMExportMethod { CD, USB };

enum ArchTypeID { ARCH_MAXILLA = 0, ARCH_MANDLBLE, ARCH_TYPE_END };

//20250212 LIN
enum SettingFileType {
	MARIADB = 1, //mariadb 설치
	IMPLANTDB,	//implantdb 설치
	MARIADB_AND_IMPLANTDB,
	WILL3D_SETUP,
	WILL3D_IMPLANT,
	VERSION_INI,
	
};

enum VersionCheckType {
	WILL3D_VER=0,
	IMPLANT_VER,
};
