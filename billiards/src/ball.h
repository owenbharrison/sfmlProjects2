#pragma once
#ifndef BALL_H
#define BALL_H

#include "geom/aabb.h"
using namespace common;

constexpr float PI=3.1415927f;

struct Ball {
	Float2 pos, vel;
	float rad=0, mass=1;

	Ball() {}

	Ball(Float2 p, Float2 v, float r) : pos(p), vel(v), rad(r) {
		mass=PI*rad*rad;
	}

	void update(float dt) {
		//drag
		//vel*=1-dt;

		pos+=vel*dt;
	}

	void checkAABB(AABB a) {
		if (pos.x<a.min.x+rad) pos.x=a.min.x+rad, vel.x*=-1;
		if (pos.y<a.min.y+rad) pos.y=a.min.y+rad, vel.y*=-1;
		if (pos.x>a.max.x-rad) pos.x=a.max.x-rad, vel.x*=-1;
		if (pos.y>a.max.y-rad) pos.y=a.max.y-rad, vel.y*=-1;
	}
};
#endif