#pragma once
#ifndef UTIL_H
#define UTIL_H

#include <cstdlib>

#include "math/float2.h"
using namespace common;

constexpr float PI=3.1415927f;

inline float rand01() {
	return rand()/float(RAND_MAX);
}

inline Float2 polarToCartesian(float rad, float angle) {
	return rad*Float2(cosf(angle), sinf(angle));
}
#endif