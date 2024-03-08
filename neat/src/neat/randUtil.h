//why not?

#pragma once
#ifndef RANDUTIL_H
#define RANDUTIL_H

#include <random>

std::mt19937_64 randGen(time(NULL));

inline float rand01() {
	static std::uniform_real_distribution<float> dist(0, 1);
	return dist(randGen);
}

inline size_t randN(size_t n) {
	static std::uniform_int_distribution<size_t> dist(0, n-1);
	return dist(randGen);
}

template<typename T>
inline T& randElem(std::vector<T>& vec) {
	return vec[randN(vec.size())];
}
#endif