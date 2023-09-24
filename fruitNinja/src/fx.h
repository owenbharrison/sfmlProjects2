#include "math/Float2.h"
using namespace common;

#pragma once
struct FX {
	Float2 pos, vel, acc;
	float age, lifespan;

	FX() : age(0), lifespan(0) {}

	FX(Float2 pos, Float2 vel, float lifespan) : pos(pos), vel(vel), age(0), lifespan(lifespan) {}

	void update(float dt) {
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;

		age+=dt;
	}

	void accelerate(Float2 f) { acc+=f; }

	bool isDead() const {
		return age>lifespan;
	}
};