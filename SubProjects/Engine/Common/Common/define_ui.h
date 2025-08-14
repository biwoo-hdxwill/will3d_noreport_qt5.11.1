#pragma once
namespace common {
namespace ui_define {
// QGraphicsItem 들의 ZValue 는 높은게 위로 온다.
const float kZValueLine = 51.0f;
const float kZValuePoint = 52.0f;
const float kZValueGuideLine = 50.0f;
const float kZValueLabel = 53.0f;
const float kZValueRuler = 55.0f;
/*
프로그램 크기 변경해도 임플란트 버튼의 크기를 변경시킬 계획이 없기 때문에 상수로 정의하였음.
*/
const int kImplantButtonWidth = 36;
const int kImplantButtonHeight = 36;
const int kToolLineWidth = 2;

const int kViewMarginWidth = 10;
const int kViewMarginHeight = 10;
const int kViewSpacing = 20;
const int kViewFilterOffsetY = 30;

} // end of namespace ui_define
} // end of namespace common
