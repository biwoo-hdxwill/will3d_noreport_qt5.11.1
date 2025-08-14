#pragma once

/**=================================================================================================

Project:		Renderer
File:			defines.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-18
Last modify: 	2018-09-18

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

namespace Renderer {
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
