#pragma once
#ifndef SLIDER_H
#define SLIDER_H

#include "../math/float2.h"

namespace common {
	inline float clamp(float x, float a, float b);

	inline float invVecLerp(Float2 pa, Float2 ba);

	struct Slider {
		Float2 range;
		Float2 start, end;
		float rad=5, t=0;
		bool down=false;

		Slider();

		Slider(Float2 r, Float2 s, Float2 e);

		float getValue() const;

		void grab(Float2 pt);

		void drag(Float2 pt);

		void release();
	};
}
#endif