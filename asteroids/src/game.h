/*NOTE: the fitness weights are arbitrary
they drive the "intended purpose" of creatures*/
#pragma once
#ifndef GAME_H
#define GAME_H

#include "game/ship.h"
#include "brain.h"
#include "game/asteroid.h"
using namespace common;

//torodial space
void wrapPos(Float2& p, const AABB a) {
	if (p.x<a.min.x) p.x=a.max.x;
	else if (p.x>a.max.x) p.x=a.min.x;
	if (p.y<a.min.y) p.y=a.max.y;
	else if (p.y>a.max.y) p.y=a.min.y;
}

Float2 ptOnEdge(const AABB a) {
	Float2 t(rand01(), rand01()<.5f);
	if (rand01()<.5f) {//swap edges
		float a=t.x; t.x=t.y, t.y=a;
	}
	return a.min+t*(a.max-a.min);
}

struct Game {
	AABB bounds;

	Ship ship;

	//clever names elude me
	static constexpr size_t sensors=16, controls=3;
	typedef Brain<sensors, controls> BrainType;
	BrainType brain;

	std::list<Asteroid> asteroids;

	float fitness=0;
	size_t lives=0;
	float spawnProt=0;
	size_t stage=0;

	Game() {}

	Game(AABB b) : bounds(b) {
		reset();
	}

	//gamestate qualifiers
	inline bool isDead() const {
		return lives<1;
	}

	inline bool hasWon() const {
		return stage>3;
	}

	//gamestate setup
	void respawn() {
		ship=Ship(.5f*(bounds.min+bounds.max), 8.5f);

		spawnProt=1.5f;
	}

	void spawnAsteroids(size_t num) {
		//initialize asteroids on edge
		for (size_t i=0; i<num; i++) {
			Float2 pos=ptOnEdge(bounds);
			float speed=10+5*rand01();
			float angle=2*PI*rand01();
			Float2 vel=speed*Float2(cosf(angle), sinf(angle));
			asteroids.push_back(Asteroid(pos, vel, 11+6*rand01()));
		}
	}

	void reset() {
		respawn();

		brain.prepare();

		asteroids.clear();
		spawnAsteroids(2);

		fitness=0;
		lives=3;
		stage=0;
	}

	//brain usage
	void think() {
		//inputs(raycast)
		const auto rad=250;
		for (size_t i=0; i<sensors; i++) {
			float angle=2*PI*i/sensors;
			Float2 dir(cosf(angle), sinf(angle));
			Float2 vB=ship.pos+rad*dir;

			float record=1;
			for (const auto& a:asteroids) {
				for (size_t j=0; j<a.numPts; j++) {
					const Float2& vC=a.outline[j], & vD=a.outline[(j+1)%a.numPts];

					Float2 tu=lineLineIntersection(ship.pos, vB, a.pos+vC, a.pos+vD);
					if (tu.x>=0&&tu.x<=1&&tu.y>=0&&tu.y<=1) {
						if (tu.x<record) record=tu.x;
					}
				}
			}

			brain.inputs[i]->value=record;
		}

		brain.propagate();
	}

	void act(float dt) {
		ship.rot+=dt*brain.outputs[0]->value;
		if (float v=brain.outputs[1]->value; v>0) {
			ship.boost(v);

			//fuel
			fitness-=dt*v;
		}
		if (brain.outputs[2]->value>0) {
			//bullet consumption
			if (ship.fire()) fitness--;
		}
	}

	//game loop
	void update(float dt) {
		//position integration
		ship.update(dt);
		wrapPos(ship.pos, bounds);
		for (auto& a:asteroids) {
			a.update(dt);
			wrapPos(a.pos, bounds);
		}

		//hit by asteroid?
		if (spawnProt<0) for (const auto& a:asteroids) {
			if (a.containsPt(ship.pos)) {
				lives--;
				fitness-=20;

				respawn();
			}
		}
		spawnProt-=dt;

		//bullet-asteroid collision detection
		auto aEnd=asteroids.end();
		for (auto bit=ship.bullets.begin(); bit!=ship.bullets.end();) {
			const auto& b=*bit;

			//delete bullet if offscreen or colliding
			bool toDel=!bounds.containsPt(b.pos);
			if (!toDel) for (auto ait=asteroids.begin(); ait!=aEnd;) {
				const auto& a=*ait;

				if (a.containsPt(b.pos)) {
					Asteroid a0, a1;
					if (a.split(a0, a1)) {
						//break into pieces
						asteroids.insert(asteroids.end(), {a0, a1});
					}

					//invlerp, then smoothstep(35 for small ast, 10 for big ast)
					float t=1-a.getRad()/Asteroid::RMax;
					t*=t*(3-2*t);
					fitness+=10+25*t;

					ait=asteroids.erase(ait);

					toDel=true;
					break;
				} else ait++;
			}

			if (toDel) bit=ship.bullets.erase(bit);
			else bit++;
		}

		//stage changes
		if (asteroids.empty()) {
			respawn();

			switch (++stage) {
				case 1: spawnAsteroids(3); break;
				case 2: spawnAsteroids(4); break;
				default: spawnAsteroids(6); break;
			}
			fitness+=250*stage;
		}

		//reward for time survived
		fitness+=dt;
	};
};
#endif