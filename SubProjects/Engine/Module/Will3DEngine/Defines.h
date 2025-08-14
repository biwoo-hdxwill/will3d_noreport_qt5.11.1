#pragma once

#include <QString>

namespace Will3DEngine {
	enum VolType {
		VOL_MAIN = 0,
		VOL_SECOND,
		VOL_PANORAMA,
		VOL_TYPE_END
	};

	struct GLdeviceInfo {
		QString vendor;
		int total_gpu_memory_kb;
		int curr_gpu_memory_kb;
		int max_tex_axis_size;
		int max_3d_tex_size;
		int max_count_texture;
		bool is_valid;
		GLdeviceInfo() : total_gpu_memory_kb(0),
			curr_gpu_memory_kb(0),
			max_tex_axis_size(0),
			max_3d_tex_size(0),
			max_count_texture(0),
			is_valid(false) {
		}
	};
}
