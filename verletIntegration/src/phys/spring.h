#pragma once
#ifndef SPRING_H
#define SPRING_H

#include "particle.h"

struct Spring {
	static const float defStiffness, defDamp;

	Particle* a=nullptr, * b=nullptr;
	float restLen=0, stiffness=0, damping=0;

	Spring() {}

	Spring(Particle& a_, Particle& b_, float k=defStiffness, float d=defDamp) : a(&a_), b(&b_), stiffness(k), damping(d) {
		restLen=length(a->pos-b->pos);
	}

	Float2 getForce(float dt){
		Float2 sub=b->pos-a->pos;
		float currLen=length(sub);
		float SpringForce=stiffness*(currLen-restLen);
		Float2 norm=sub/currLen;
		Float2 aVel=(a->pos-a->oldpos)/dt;
		Float2 bVel=(b->pos-b->oldpos)/dt;
		float dampForce=damping*dot(norm, bVel-aVel);
		return norm*(SpringForce+dampForce);
	}

	void update(float dt) {
		Float2 force=getForce(dt);
		if(!a->locked) a->applyForce(force);
		if(!b->locked) b->applyForce(-force);
	}
};
const float Spring::defStiffness=560.3f*(PI*Particle::defRad*Particle::defRad);
const float Spring::defDamp=4.3f*(PI*Particle::defRad*Particle::defRad);
#endif