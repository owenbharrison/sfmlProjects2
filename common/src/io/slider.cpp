#include "slider.h"

namespace common {
	inline float clamp(float x, float a, float b) {
		if (x<a) return a;
		if (x>b) return b;
		return x;
	}

	inline float invVecLerp(Float2 pa, Float2 ba) {
		return clamp(dot(pa, ba)/dot(ba, ba), 0, 1);
	}

	Slider::Slider() {}

	Slider::Slider(Float2 r, Float2 s, Float2 e) : range(r), start(s), end(e) {}

	float Slider::getValue() const {
		return range.x+t*(range.y-range.x);
	}

	void Slider::grab(Float2 pt) {
		Float2 pa=pt-start, ba=end-start;
		float tNew=invVecLerp(pa, ba);
		if (length(pa-tNew*ba)<rad) {
			t=tNew;
			down=true;
		} else down=false;
	}

	void Slider::drag(Float2 pt) {
		if (down) {
			Float2 pa=pt-start, ba=end-start;
			t=invVecLerp(pa, ba);
		}
	}

	void Slider::release() {
		down=false;
	}
}