#pragma once
#ifndef SOUND_H
#define SOUND_H

#include "math/float2.h"
using namespace common;

struct Sound {
	Float2 pos;
	float rad=0, maxRad=100;
	const float speed=37.2f;

	inline void update(float dt) {
		rad+=dt*speed;
	}

	inline bool isDead() const {
		return rad>maxRad;
	}
};
#endif