#pragma once
#ifndef PTC_H
#define PTC_H

#include "geom/aabb.h"
using namespace common;

#define PI 3.1415927f

struct ptc {
	static const float defaultRad;
	float2 pos, oldpos, forces;
	float rad, mass;

	ptc() : rad(0), mass(1) {}

	ptc(float2 p, float r=defaultRad) : pos(p), oldpos(p), rad(r) {
		mass=PI*rad*rad;
	}

	void applyForce(float2 f) {
		forces+=f;
	}

	void update(float dt) {
		float2 vel=pos-oldpos;
		oldpos=pos;

		float2 acc=forces/mass;
		pos+=vel+acc*dt*dt;

		forces*=0;
	}

	void checkAABB(aabb a) {
		float2 vel=pos-oldpos;
		if (pos.x<a.min.x+rad) pos.x=a.min.x+rad, oldpos.x=pos.x+vel.x;
		if (pos.y<a.min.y+rad) pos.y=a.min.y+rad, oldpos.y=pos.y+vel.y;
		if (pos.x>a.max.x-rad) pos.x=a.max.x-rad, oldpos.x=pos.x+vel.x;
		if (pos.y>a.max.y-rad) pos.y=a.max.y-rad, oldpos.y=pos.y+vel.y;
	}
};

const float ptc::defaultRad=8.23f;
#endif