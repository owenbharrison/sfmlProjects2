#pragma once
#ifndef ASTEROID_H
#define ASTEROID_H

#include "util.h"

struct Asteroid {
	Float2 pos, vel;
	size_t numPoints;
	Float2* outline;

	Asteroid() : numPoints(0), outline(nullptr) {}

	Asteroid(Float2 p, Float2 v, size_t n, float nr, float xr) : pos(p), vel(v), numPoints(n) {
		outline=new Float2[numPoints];
		for (size_t i=0; i<numPoints; i++) {
			float rad=nr+rand01()*(xr-nr);
			float angle=2*PI*i/numPoints;
			outline[i]=polarToCartesian(rad, angle);
		}
	}

	Asteroid(const Asteroid& a) : pos(a.pos), vel(a.vel), numPoints(a.numPoints) {
		outline=new Float2[numPoints];
		memcpy(outline, a.outline, sizeof(Float2)*numPoints);
	}

	~Asteroid() {
		delete[] outline;
	}

	Asteroid& operator=(const Asteroid& a) {
		if (this==&a) return *this;

		delete[] outline;

		pos=a.pos;
		vel=a.vel;
		numPoints=a.numPoints;
		outline=new Float2[numPoints];
		memcpy(outline, a.outline, sizeof(Float2)*numPoints);

		return *this;
	}

	void update(float dt) {
		pos+=vel*dt;
	}

	//polygon raycasting method
	bool containsPt(Float2 pt) {
		pt-=pos;//make point local

		size_t num=0;
		for (size_t i=0; i<numPoints; i++) {
			Float2 a=outline[i], b=outline[(i+1)%numPoints];
			Float2 tu=lineLineIntersection(a, b, pt, pt+Float2(1, 0));
			if (tu.x>=0&&tu.x<=1&&tu.y>=0) num++;
		}

		//odd num ix
		return num%2;
	}

	bool split(Asteroid& a, Asteroid& b) {
		float totalRad=0;
		for (size_t i=0; i<numPoints; i++) {
			totalRad+=length(outline[i]);
		}
		float avgRad=totalRad/numPoints;
		if (avgRad<15) return false;

		Float2 tang(-vel.y, vel.x);
		tang*=1+.5f*rand01();
		size_t num=numPoints-2; if (num<5) num=5;
		a=Asteroid(pos, tang, num, avgRad*.5f, avgRad*.8f);
		b=Asteroid(pos, -tang, num, avgRad*.5f, avgRad*.8f);
		return true;
	}
};
#endif