#ifndef VRENGINE_GLOBAL_H
#define VRENGINE_GLOBAL_H

#include <QtCore/qglobal.h>
#ifdef VRENGINE_LIB
# define VRENGINE_EXPORT Q_DECL_EXPORT
#else
# define VRENGINE_EXPORT Q_DECL_IMPORT
#endif

namespace VREngine
{
	enum VolType
	{
		MAIN,
		SECOND,
		PANORAMA,
		VOL_TYPE_END
	};


	const unsigned int kTexNumDefalut = 0x84C0; //GL_TEXTURE0
	const int kTexNumDefalut_ = 0;

	const unsigned int kTexNumVol = 0x84C1; //GL_TEXTURE1
	const int kTexNumVol_ = 1;

	const unsigned int kTexNumTF = 0x84C2; //GL_TEXTURE2
	const int kTexNumTF_ = 2;

	const unsigned int kTexNumVolSecond = 0x84C3; //GL_TEXTURE3
	const int kTexNumVolSecond_ = 3;

	const unsigned int kTexNumVolPano = 0x84C4; //GL_TEXTURE4
	const int kTexNumVolPano_ = 4;

	const unsigned int kTexNumEnd = 0x84C5; //GL_TEXTURE5
	const int kTexNumEnd_ = 5;


	class WindowWL {
	public:

		WindowWL() { }
		WindowWL(float width, float level) {
			this->width_ = width;
			this->level_ = level;
		}

		WindowWL(const WindowWL& wwl) { *this = wwl; }
		WindowWL& operator=(const WindowWL& wwl) {
			this->width_ = wwl.width_;
			this->level_ = wwl.level_;
			this->delta_width_ = wwl.delta_width_;
			this->delta_level_ = wwl.delta_level_;
			return *this;
		}

		inline void GetAdjustedWL(float& width, float& level) const { width = width_ + delta_width_; level = level_ + delta_level_; }
		inline float GetAdjustedWidth() const { return width_ + delta_width_; }
		inline float GetAdjustedLevel() const { return level_ + delta_level_; }

		inline void set_width(const float& width) { width_ = width; }
		inline void set_level(const float& level) { level_ = level; }
		inline void set_delta_width(const float& delta_width) { delta_width_ = delta_width; }
		inline void set_delta_level(const float& delta_level) { delta_level_ = delta_level; }

		inline const float& width() const { return width_; }
		inline const float& level() const { return level_; }
		inline const float& delta_width() const { return delta_width_; }
		inline const float& delta_level() const { return delta_level_; }
	private:

		float width_ = 0.0f;
		float level_ = 0.0f;
		float delta_width_ = 0.0f;
		float delta_level_ = 0.0f;
	};
}


#endif // VRENGINE_GLOBAL_H
