#include "geom/AABB.h"
using namespace common;

#define PI 3.1415927f

#pragma once
struct Particle {
	Float2 pos, oldpos, force, oldforce;
	float rad=0, mass=0;
	int id=-1;
	bool locked=false;
	bool ghosted=false;

	Particle() {}

	Particle(Float2 pos, float rad) : pos(pos), oldpos(pos), rad(rad) {
		mass=PI*rad*rad;
	}

	void update(float dt) {
		Float2 vel=pos-oldpos;
		//save pos
		oldpos=pos;
		oldforce=force;
		//verlet integrate
		if (!locked) pos+=vel+force/mass*dt*dt;
		//reset forces
		force*=0;
	}

	void applyForce(Float2 f) {
		if (!locked) force+=f;
	}

	void checkAABB(AABB a) {
		Float2 vel=pos-oldpos;
		if (pos.x<a.min.x+rad) pos.x=a.min.x+rad, oldpos.x=pos.x+vel.x;
		if (pos.y<a.min.y+rad) pos.y=a.min.y+rad, oldpos.y=pos.y+vel.y;
		if (pos.x>a.max.x-rad) pos.x=a.max.x-rad, oldpos.x=pos.x+vel.x;
		if (pos.y>a.max.y-rad) pos.y=a.max.y-rad, oldpos.y=pos.y+vel.y;
	}
};