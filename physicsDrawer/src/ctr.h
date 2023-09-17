#pragma once
#ifndef CTR_H
#define CTR_H

#include "ptc.h"

struct ctr {
	ptc* a, * b;
	float restLen;

	ctr() : a(nullptr), b(nullptr), restLen(0) {}

	ctr(ptc& a, ptc& b) : a(&a), b(&b) {
		restLen=length(a.pos-b.pos);
	}
};
#endif