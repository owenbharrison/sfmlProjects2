#pragma once
#ifndef SPR_H
#define SPR_H

#include "ctr.h"

struct spr : ctr {
	static const float defaultStiffness, defaultDamping;
	float stiffness, damping;

	spr() : ctr(), stiffness(0), damping(0) {}

	spr(ptc& a, ptc& b, float k=defaultStiffness, float d=defaultDamping) : ctr(a, b), stiffness(k), damping(d) {}

	float2 getForce(float dt) const {
		float2 sub=b->pos-a->pos;
		float currLen=length(sub);
		float sprForce=stiffness*(currLen-restLen);
		float2 norm=sub/currLen;
		float2 aVel=(a->pos-a->oldpos)/dt;
		float2 bVel=(b->pos-b->oldpos)/dt;
		float dampForce=damping*dot(norm, bVel-aVel);
		return norm*(sprForce+dampForce);
	}

	void update(float dt) {
		float2 f=getForce(dt);
		a->applyForce(f);
		b->applyForce(-f);
	}
};

const float spr::defaultStiffness=57051.63f, spr::defaultDamping=645.1f;
#endif