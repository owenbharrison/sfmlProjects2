#pragma once
#ifndef AABB_H
#define AABB_H

#include "../math/float2.h"

namespace common{
	inline float MIN(float a, float b);
	inline float MAX(float a, float b);
	inline float2 MIN(float2 a, float2 b);
	inline float2 MIN(float2 a, float2 b);

	struct aabb {
		float2 min, max;

		aabb();
		aabb(float2 a, float2 b);

		//is point in bounds
		bool containsPt(float2 pt) const;

		//rectangular overlap
		bool overlaps(aabb a) const;

		//returns whether to show line
		bool clipLine(float2& a, float2& b) const;
	};
}
#endif