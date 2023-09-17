#pragma once
#ifndef CST_H
#define CST_H

#include "ctr.h"

struct cst : ctr {
	static const float defaultRad;
	float rad;

	cst() : ctr(), rad(0) {}

	cst(ptc& a, ptc& b, float rad=defaultRad) : ctr(a, b), rad(rad) {}

	float2 getCorrection() const {
		float2 axis=a->pos-b->pos;
		float dist=length(axis);
		float2 norm=axis/dist;
		float diff=restLen-dist;
		return norm*diff;
	}

	void update() {
		float2 f=getCorrection()/(a->mass+b->mass);
		a->pos+=b->mass*f;
		b->pos-=a->mass*f;
	}
};

const float cst::defaultRad=5.37f;
#endif