#pragma once
#ifndef PARTICLE_H
#define PARTICLE_H

#include "geom/aabb.h"
using namespace common;

struct Particle {
	Float2 pos, vel;
	float rad, age=0, lifespan;

	Particle() : rad(0), lifespan(0) {}

	Particle(Float2 p, Float2 v, float r, float l) : pos(p), vel(v), rad(r), lifespan(l) {}

	void update(float dt) {
		vel*=1-dt;
		pos+=vel*dt;

		age+=dt;
	}

	bool isValid(AABB a) const {
		return a.containsPt(pos)&&age<lifespan;
	}
};
#endif