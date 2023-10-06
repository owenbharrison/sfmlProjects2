#pragma once
#ifndef SHIP_H
#define SHIP_H

#include "util.h"

#include "particle.h"

struct Ship {
	Float2 pos, vel, acc;
	float rad=0, rot=0;

	Ship() {}

	Ship(Float2 p, float r) : pos(p), rad(r) {}

	void update(float dt) {
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	void boost(float f) {
		acc+=polarToCartesian(f, rot);
	}

	void turn(float f) {
		rot+=f;
	}

	Particle emitParticle() const {
		float angle=rot+PI+.45f*PI*(.5f-rand01());
		Float2 pPos=pos+polarToCartesian(.5f*rad, angle);
		float speed=10*5*rand01();
		float pRad=.7f+.8f*rand01();
		float lifespan=.6f+.4f*rand01();
		return Particle(pPos, polarToCartesian(speed, angle), pRad, lifespan);
	}
};
#endif