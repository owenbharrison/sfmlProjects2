#include "math/float2.h"
using namespace common;

#pragma once
struct fx {
	float2 pos, vel, acc;
	float age, lifespan;

	fx() : age(0), lifespan(0) {}

	fx(float2 pos, float2 vel, float lifespan) : pos(pos), vel(vel), age(0), lifespan(lifespan) {}

	void update(float dt) {
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;

		age+=dt;
	}

	void accelerate(float2 f) { acc+=f; }

	bool isDead() const {
		return age>lifespan;
	}
};