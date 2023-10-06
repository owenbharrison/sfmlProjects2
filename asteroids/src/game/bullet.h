#pragma once
#ifndef BULLET_H
#define BULLET_H

#include "geom/aabb.h"
using namespace common;

struct Bullet {
	Float2 pos, vel;

	Bullet() {}

	Bullet(Float2 pos, Float2 vel) : pos(pos), vel(vel) {}

	void update(float dt) {
		pos+=vel*dt;
	}

	bool isValid(AABB a) const {
		return a.containsPt(pos);
	}
};
#endif