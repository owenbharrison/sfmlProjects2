#pragma once
#ifndef SHIP_H
#define SHIP_H

#include <list>

#include "util.h"
using namespace common;

struct Ship {
	Float2 pos, vel, acc;
	float rad=0, rot=0;

	struct Bullet {
		Float2 pos, vel;

		inline void update(float dt) {
			pos+=dt*vel;
		}
	};
	std::list<Bullet> bullets;
	float bulletTimer=0;

	Ship() {}

	Ship(Float2 p, float r) : pos(p), rad(r) {}

	void update(float dt) {
		vel+=dt*acc;
		pos+=dt*vel;
		acc*=0;

		for (auto& b:bullets) b.update(dt);

		bulletTimer-=dt;
	}

	inline void boost(float amt) {
		acc+=amt*Float2(cosf(rot), sinf(rot));
	}

	inline void turn(float f) {
		rot+=f;
	}

	bool fire() {
		//we can only shoot every so often
		if (bulletTimer<0) {
			bulletTimer=1.25f;

#if 1//enable shotgun
			for (size_t i=0; i<2; i++) {
				float speed=175+55*rand01();
				float angle=rot+.12f*PI*(.5f-rand01());
				Float2 vel=speed*Float2(cosf(angle), sinf(angle));
				bullets.push_back({pos, vel});

				//recoil
				boost(-speed);
			}
#else
			float speed=175+55*rand01();
			Float2 vel=speed*Float2(cosf(rot), sinf(rot));
			bullets.push_back({pos, vel});
#endif
			return true;
		}

		return false;
	}
};
#endif