#pragma once
/*=========================================================================

File:		colletion of define statements
Language:	C++11
Library:	Standard C++ Library

=========================================================================*/
#define MARIA_DB 1
#define MAX_IMPLANT	28

#define CHINA_VERSION 0

namespace common {
const int kVersionCurrent = 1200;
const int kMaxLightbox = 20;
const float kMPRThicknessMax = 50.0f;
const float kArcballSensitivity = 1.8f;

namespace dicom {
const int kFreeFOVSliceMaximumSizeMM = 6400; //8cm*8cm. 일반적으로 5cm*5cm를 사용한다고 함.
const float kInvalidHU = std::numeric_limits<short>::min();
} // end of namespace dicom
} // end of namespace common
