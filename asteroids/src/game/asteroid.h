#pragma once
#ifndef ASTEROID_H
#define ASTEROID_H

#include "util.h"
#include "geom/aabb.h"
using namespace common;

float clamp(float x, float a, float b) {
	if (x<a) return a;
	if (x>b) return b;
	return x;
}

//rule of three!!!
struct Asteroid {
	static constexpr float RMax=50.3f;
	Float2 pos, vel;
	size_t numPts=0;
	Float2* outline=nullptr;

	Asteroid() {}

	Asteroid(Float2 p, Float2 v, size_t n, float nr=26.7f, float xr=RMax) : pos(p), vel(v), numPts(n) {
		outline=new Float2[numPts];

		for (size_t i=0; i<numPts; i++) {
			float rad=nr+rand01()*(xr-nr);
			float angle=2*PI*i/numPts;
			outline[i]=rad*Float2(cosf(angle), sinf(angle));
		}
	}

	Asteroid(const Asteroid& a) : pos(a.pos), vel(a.vel), numPts(a.numPts) {
		outline=new Float2[numPts];
		memcpy(outline, a.outline, sizeof(Float2)*numPts);
	}

	~Asteroid() {
		delete[] outline;
	}

	Asteroid& operator=(const Asteroid& a) {
		if (this==&a) return *this;

		delete[] outline;

		pos=a.pos;
		vel=a.vel;
		numPts=a.numPts;
		outline=new Float2[numPts];
		memcpy(outline, a.outline, sizeof(Float2)*numPts);

		return *this;
	}

	inline void update(float dt) {
		pos+=dt*vel;
	}

	float getRad() const {
		float rSum=0;
		for (size_t i=0; i<numPts; i++) {
			rSum+=length(outline[i]);
		}
		return rSum/numPts;
	}

	bool split(Asteroid& a, Asteroid& b) const {
		float rAvg=getRad();
		if (rAvg<20) return false;

		//speed up by 1.1-1.25x
		Float2 tang(-vel.y, vel.x);
		tang*=1.1f+.15f*rand01();

		//min asteroid size
		size_t num=numPts-2; if (num<4) num=4;

		//two smaller going opposite dirs
		a=Asteroid(pos, tang, num, rAvg*.7f, rAvg*.85f);
		b=Asteroid(pos, -tang, num, rAvg*.7f, rAvg*.85f);

		return true;
	}

	bool containsPt(Float2 pt) const {
		pt-=pos;//make point local

		size_t num=0;
		for (size_t i=0; i<numPts; i++) {
			Float2 a=outline[i], b=outline[(i+1)%numPts];
			Float2 tu=lineLineIntersection(a, b, pt, pt+Float2(1, 0));
			if (tu.x>=0&&tu.x<=1&&tu.y>=0) num++;
		}

		//odd num ix
		return num%2;
	}
};
#endif