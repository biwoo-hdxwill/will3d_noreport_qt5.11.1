#pragma once
#include <Engine/Common/Common/W3Enum.h>

typedef struct _ClipStatus {
	MPRClipID plane;
	bool enable;
	bool flip;
	int lower;
	int upper;
} ClipStatus;
