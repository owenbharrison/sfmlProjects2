#pragma once
#ifndef PHYSICSSOLVER_H
#define PHYSICSSOLVER_H

#include <list>
#include <vector>

#include "constraint.h"
#include "spring.h"

class PhysicsSolver {
	std::list<Particle>* particles=nullptr;
	std::list<Constraint>* constraints=nullptr;
	std::list<Spring>* springs=nullptr;

	Float2 gravity;
	bool useGravity=false;

	float dragCoeff=0;
	bool useDrag=false;

	AABB boundary;
	bool useBoundary=false;

	float cellSize=0;
	size_t numCellX=1, numCellY=1;
	std::vector<Particle*>* spacialHash=nullptr;
	bool useSpacialHash=false;

public:
	PhysicsSolver() {}

	PhysicsSolver(std::list<Particle>* p, std::list<Constraint>* c, std::list<Spring>* s) : particles(p), constraints(c), springs(s) {}

	~PhysicsSolver() {
		delete[] spacialHash;
	}

	void initGravity(Float2 g) {
		gravity=g;
		useGravity=true;
	}

	void initDrag(float cd) {
		dragCoeff=cd;
		useDrag=true;
	}

	void initBounds(AABB& b) {
		boundary=b;
		useBoundary=true;
	}

	[[nodiscard]] bool initSpacialHash(float sz) {
		if (!useBoundary) return false;
		if (sz<=0) return false;

		cellSize=sz;
		Float2 cellNum=(boundary.max-boundary.min)/sz;
		numCellX=ceilf(cellNum.x), numCellY=ceilf(cellNum.y);
		spacialHash=new std::vector<Particle*>[numCellX*numCellY];
		useSpacialHash=true;
		return true;
	}

	void update(float dt) {
		//connectors
		if (constraints) for (auto& c:*constraints) c.update();

		if (springs) for (auto& s:*springs) s.update(dt);

		//collisions
		ParticleParticleCollisions();

		ParticleConstraintCollisions();

		//dynamics
		if (particles) for (auto& p:*particles) {
			//f=mg
			if (useGravity) p.applyForce(p.mass*gravity);

			if (useDrag) {
				//f=-cv^2
				Float2 vel=(p.pos-p.oldpos)/dt;
				p.applyForce(-dragCoeff*vel*length(vel));
			}

			//"integrate"
			p.update(dt);

			//keep in box
			if (useBoundary) p.checkAABB(boundary);
		}
	}

	void ParticleParticleCollide(Particle& a, Particle& b) {
		//narrow phase
		Constraint cTemp(a, b, 0);
		float totalRad=a.rad+b.rad;
		if (cTemp.restLen>totalRad) return;

		//resolution
		cTemp.restLen=totalRad;
		cTemp.update();
	}

	void ParticleParticleCollisions() {
		if (!particles) return;

		if (useSpacialHash) {
			//clear buckets
			for (size_t i=0; i<numCellX*numCellY; i++) spacialHash[i].clear();

			//fill buckets
			for (auto& p:*particles) {
				if (p.ghosted) continue;

				//determine cell
				Float2 xy=(p.pos-boundary.min)/cellSize;
				int i=xy.x, j=xy.y;

				//is cell invalid	
				if (i<0||i>=numCellX||j<0||j>=numCellY) continue;

				//add to cell
				spacialHash[i+j*numCellX].push_back(&p);
			}

			//check all buckets
			for (size_t x=0; x<numCellX; x++) {
				for (size_t y=0; y<numCellY; y++) {
					auto& curr=spacialHash[x+y*numCellX];

					//check all neighbors
					for (int dx=-1; dx<=1; dx++) {
						for (int dy=-1; dy<=1; dy++) {
							int i=x+dx, j=y+dy;

							//is cell valid
							if (i<0||i>=numCellX||j<0||j>=numCellY) continue;

							auto& adj=spacialHash[i+j*numCellX];

							//query both containers
							for (auto& a:curr) {
								for (auto& b:adj) {
									//dont check self
									if (a==b) continue;

									ParticleParticleCollide(*a, *b);
								}
							}
						}
					}
				}
			}
		} else {
			//O(n^2) approach
			for (auto ait=particles->begin(); ait!=particles->end(); ait++) {
				auto& a=*ait;
				if (a.ghosted) continue;

				for (auto bit=next(ait); bit!=particles->end(); bit++) {
					auto& b=*bit;
					if (b.ghosted) continue;

					ParticleParticleCollide(a, b);
				}
			}
		}
	}

	void ParticleConstraintCollisions() {
		if (!particles||!constraints) return;

		//for every Particle
		for (auto& p:*particles) {
			if (p.ghosted) continue;

			//check every Constraint
			for (auto& c:*constraints) {
				if (c.ghosted) continue;

				//dont check "self"
				if (&p==c.a||&p==c.b) continue;

				//where along the Constraint is the point
				Float2 pa=p.pos-c.a->pos, ba=c.b->pos-c.a->pos;
				float t=dot(pa, ba)/dot(ba, ba);
				if (t<0) { t=0; } if (t>1) { t=1; }

				//temporary Particle for updating
				Particle tempP(c.a->pos+ba*t, 0);
				float totalRad=p.rad+c.rad;
				Constraint tempC(p, tempP, 0);

				//if intrinsic dist is overlapping
				if (tempC.restLen>totalRad) continue;

				//resolve
				tempC.restLen=totalRad;
				float cMass=c.a->mass+c.b->mass;
				float totalMass=p.mass+cMass;
				Float2 f=tempC.getForce()/totalMass;
				if (!p.locked) p.pos+=cMass*f;
				if (!c.a->locked) c.a->pos-=(1-t)*(c.b->mass/cMass)*p.mass*f;
				if (!c.b->locked) c.b->pos-=t*(c.a->mass/cMass)*p.mass*f;
			}
		}
	}
};
#endif