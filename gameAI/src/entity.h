#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include "sound.h"

struct Entity {
	Float2 pos;
	float rot=0;

	void move(float s) {
		Float2 dir(cosf(rot), sinf(rot));
		pos+=s*dir;
	}

	void turn(float r) {
		rot+=r;
	}
};
#endif