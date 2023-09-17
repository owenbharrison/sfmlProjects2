#include <vector>

#include "geom/aabb.h"
using namespace common;

float2 rotVec(float2 p, float a) {
	float c=cosf(a);
	float s=sinf(a);
	return float2(
		p.x*c+p.y*s,
		p.y*c-p.x*s
	);
}

#pragma once
struct fruit {
	float2 pos, vel, acc;
	float rot, rotVel;
	bool sliced=false;
	std::vector<float2> pts;

	fruit() : rot(0), rotVel(0) {}

	fruit(float2 pos, float2 vel, float rot, float rotVel) : pos(pos), vel(vel), rot(rot), rotVel(rotVel) {}

	void update(float dt) {
		rot+=rotVel*dt;

		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	void accelerate(float2 f) { acc+=f; }

	float2 localize(float2 pt) const {
		return rotVec(pt-pos, -rot);
	}

	float2 globalize(float2 pt) const {
		return pos+rotVec(pt, rot);
	}

	aabb getLocalizedAABB() const {
		aabb a;
		for (int i=0; i<pts.size(); i++) {
			const auto& p=pts.at(i);
			if (i==0||p.x<a.min.x) a.min.x=p.x;
			if (i==0||p.y<a.min.y) a.min.y=p.y;
			if (i==0||p.x>a.max.x) a.max.x=p.x;
			if (i==0||p.y>a.max.y) a.max.y=p.y;
		}
		return a;
	}

	aabb getAABB() const {
		aabb a(INFINITY, -INFINITY);
		for (int i=0; i<pts.size(); i++) {
			const auto& p=globalize(pts.at(i));
			if (i==0||p.x<a.min.x) a.min.x=p.x;
			if (i==0||p.y<a.min.y) a.min.y=p.y;
			if (i==0||p.x>a.max.x) a.max.x=p.x;
			if (i==0||p.y>a.max.y) a.max.y=p.y;
		}
		return a;
	}

	float2 getLocalizedAvg() const {
		float2 avg;
		for (const auto& p:pts) avg+=p;
		int len=pts.size();
		return avg/len;
	}

	float getArea() const {
		float2 avg=getLocalizedAvg();
		//split shape into a triangle fan
		float sum=0;
		for (int i=0, len=pts.size(); i<len; i++) {
			float2 a=pts[i];
			float2 b=pts[(i+1)%len];
			sum+=.5f*abs(a.x*(b.y-avg.y)+b.x*(avg.y-a.y)+avg.x*(a.y-b.y));
		}
		return sum;
	}

	bool checkPt(float2 pt) const {
		return checkLocalizedPt(localize(pt));
	}

	bool checkLocalizedPt(float2 pt) const {
		//if not in bounds, def not in shell
		if (!getLocalizedAABB().containsPt(pt)) return false;

		//optimized raycasting method
		int numIx=0;
		for (int i=0, len=pts.size(); i<len; i++) {
			float2 a=pts[i];
			float2 b=pts[(i+1)%len];
			float2 c=pt, d(c.x+1, c.y);
			float2 tu=lineLineIntersection(a, b, c, d);
			if (tu.x>=0&&tu.x<=1&&tu.y>=0) numIx++;
		}

		return numIx%2==1;
	}
};