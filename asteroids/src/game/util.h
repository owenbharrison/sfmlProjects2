//why not
#pragma once
#ifndef UTIL_H
#define UTIL_H

#include <random>

#include "math/float2.h"

static const float PI=3.1415927f;

inline float rand01() {
	static std::mt19937_64 generator(time(NULL));
	static std::uniform_real_distribution<float> dist(0, 1);
	return dist(generator);
}
#endif