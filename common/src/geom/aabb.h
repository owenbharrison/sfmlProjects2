#pragma once
#ifndef AABB_H
#define AABB_H

#include "../math/Float2.h"

namespace common{
	inline float MIN(float a, float b);
	inline float MAX(float a, float b);

	struct AABB {
		Float2 min, max;

		AABB();
		AABB(Float2 a, Float2 b);

		//is point in bounds
		bool containsPt(Float2 pt) const;

		//rectangular overlap
		bool overlaps(AABB a) const;

		//returns whether to show line
		bool clipLine(Float2& a, Float2& b) const;
	};
}
#endif