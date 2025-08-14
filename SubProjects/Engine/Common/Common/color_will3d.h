#pragma once

#include <QColor>

namespace ColorView {
const QColor k3D("#FFCCCCCC");
const QColor kAxial("#FFCC483A");
const QColor kCoronal("#FFAF58B7");
const QColor kSagittal("#FFCCB409");
const QColor kCrossSection("#FF29CC6E");
const QColor kCrossSectionSelected("#FFCC6E00");
const QColor kTMJFrontal("#FF2873A7");
const QColor kTMJLateral("#FF29CC6E");
const QColor kTMJLateralSelected("#FFC7AF0F");
const QColor kPanorama("#FF2873A7");
}

namespace AxisColor {
const QColor kX(245, 59, 40);
const QColor kY(0, 169, 99);
const QColor kZ(0, 147, 189);
}

namespace ColorArchItem {
	const QColor kControlBrush = QColor(0, 0, 0);
	const QColor kControlPen = QColor(0, 228, 255);
	const QColor kCurvePen = QColor(50, 151, 219);
	const QColor kCurveThicknessPen = QColor(50, 151, 219, 80);
	const QColor kCurveRangePen = QColor("#FF82735C");
}

namespace ColorTmjItem {
	const QColor kROIpen("#7C7F7F7F");
	const QColor kLateralLineNormal("#7C29CC6E");
	const QColor kLineHighlight("#7CC7AF0F");
	const QColor kFrontalLineNormal("#7C2873A7");
	const QColor kFrontalEllBrush("#FFFF0000");
	const QColor kAngleArcRight("#FF00ffff");	
	const QColor kAngleArcLeft("#FFff00ff");
	const QColor kCriterionLine("#FF7F7Fff");
	const QColor kCriterionEllBrush("#FFFF0000");
	const QColor kAxialLine("#7CFF0000");
}

namespace ColorNerveItem {
	const QColor kLinePenColor = QColor(255, 128, 0);
	const QColor kEllipsePenColor = QColor(255, 255, 255);
	const QColor kEllipseBrushColor = QColor(255, 128, 0);
}

namespace ColorCrossSectionItem {
	const QColor kNormal("#7C29CC6E");
	const QColor kHighlight("#7CCC6E00");
}

namespace ColorAxialItem {
	const QColor kLinePenColor = ("#7CCC483A");
}

namespace ColorPanoItem {
	const QColor kLienPenColor("#7C2873A7");
}

namespace ColorNerve {
	const float kAlpha = 0.05f;
}

namespace ColorImplant {
	const QColor kPanelBackGround = QColor("#FF1C1E28");
	const QColor kImplantBtnBorder = QColor("#FF000000");
	const QColor kImplantNormal = QColor("#FF434961"); // Implant button에만 적용되는 색상
	const QColor kImplantNormalHover = QColor("#FF5A6282");
#if 1
	const QColor kImplantSelected = QColor("#FF1ACBCB"); // Implant button 및 2D Rendering 에 적용되는 색상
	const QColor kImplantSelectedHover = QColor("#FF21FFFF");
	const QColor kImplantPlaced = QColor("#FF3285C6"); // Implant button 및 2D Rendering 에 적용되는 색상
	const QColor kImplantPlacedHover = QColor("#FF40B1FF");
#else
	const QColor kImplantSelected = QColor("#FF00DD00"); // Implant button 및 2D Rendering 에 적용되는 색상
	const QColor kImplantSelectedHover = QColor("#FF00FF00");
	const QColor kImplantPlaced = QColor("#FFDDDD00"); // Implant button 및 2D Rendering 에 적용되는 색상
	const QColor kImplantPlacedHover = QColor("#FFFFFF00");
#endif
	const QColor kImplantCollided = QColor("#FFFF0000"); // Implant button 및 2D Rendering 에 적용되는 색상
}

namespace ColorToolButton {
	const QColor kNormal = QColor("#FF434961");
	const QColor kHover = QColor("#FF5A6282");
	const QColor kPressed = QColor("#FF1C1E28");
	const QColor kChecked = QColor("#FF1C1E28");
	const QColor kUnchecked = QColor("#FF434961");
	const QColor kDiabled = QColor("#FF2F3243");
}

namespace ColorGeneral {
	const QColor kAccent = QColor("#FF2192EC");
	const QColor kBorder = QColor("#FF0E1017");
	const QColor kCaption = QColor("#FF1C1E28");
	const QColor kBackground = QColor("#FF2F3243");
	const QColor kBackgroundHighlight = QColor("#FF40465F");
	const QColor kTableWidgetBackground = QColor("#FFFFFFFF");
	const QColor kTableWidgetBorder = QColor("#FF0E1017");
	const QColor kTableWidgetText = QColor("#FF000000");
	const QColor kTableWidgetSelectedItem = QColor("#FFCDD0D9");
}

namespace ColorSelectVolumeRangeBar {
	const QColor kBackground = QColor("#FFFFFFFF");
	const QColor kSelectedBackground = QColor("#FF2192EC");
	const QColor kHandle = QColor("#FF424963");
}

namespace ColorTitleBar {
	const QColor kTitleBar = QColor("#FF2F3243");
}

namespace ColorGlobalPreferencesDialog {
	const QColor kUnselectedTab = QColor("#FF2F3243");
	const QColor kSelectedTab = QColor("#FF434961");
	const QColor kLine = QColor("#FFA1A4B0");
	const QColor kButtonNormal = QColor("#FF545B79");
}

namespace ColorLoginDialog {
	const QColor kBackground = QColor("#FFFFFFFF");
	//const QColor kComponentText = QColor("#FF5F5F5F");
	const QColor kComponentText = QColor("#FF000000");
	const QColor kForgotUsernamePasswordText = QColor("#FF32B345");
	const QColor kForgotUsernamePasswordTextHover = QColor("#FF46C759");
	const QColor kForgotUsernamePasswordTextPressed = QColor("#FF1E9F31");
	const QColor kUnselectedTabText = QColor("#FFA5A5A5");
	const QColor kUnselectedTabIndicator = QColor("#FFF2F2F2");
	const QColor kSelectedTabText = QColor("#FF262626");
	const QColor kSelectedTabIndicator = QColor("#FF2F4770");
	const QColor kFrameBorder = QColor("#FF000000");
	const QColor kComponentBorder = QColor("#FF808080");
}

namespace ColorTabWest {
	const QColor kPane = QColor("#FF1C1E28");
	const QColor kSelectedPane = QColor("#FF393E52");
}

namespace ColorToolBar {
	const QColor kToolBar = QColor("#FF292D39");
}

namespace ColorToolOTFButton {
	const QColor kCheckBorder = QColor("#FF2192EC");
	const QColor kUncheckBorder = QColor("#FF7E7E7E");
}

namespace ColorNavigator {
	const QColor kColor = QColor("#FFFFFFFF");
}
