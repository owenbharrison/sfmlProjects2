#pragma once
#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "particle.h"

struct Constraint {
	static const float defRad;

	Particle* a=nullptr, * b=nullptr;
	float rad=0, restLen=0;
	bool ghosted=false;

	Constraint() {}

	Constraint(Particle& a_, Particle& b_, float rad=defRad) : a(&a_), b(&b_), rad(rad) {
		restLen=length(a->pos-b->pos);
	}

	Float2 getForce() {
		Float2 axis=a->pos-b->pos;
		float dist=length(axis);
		Float2 n=axis/dist;
		float delta=restLen-dist;
		return n*delta*.5f;
	}

	void update() {
		Float2 f=getForce();
		float totalMass=a->mass+b->mass;
		if (!a->locked) a->pos+=b->mass/totalMass*2*f;
		if (!b->locked) b->pos-=a->mass/totalMass*2*f;
	}
};
const float Constraint::defRad=4.87f;
#endif