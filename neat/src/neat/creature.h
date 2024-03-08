#pragma once
#ifndef CREATURE_H
#define CREATURE_H

#include "brain.h"

template<size_t In>
struct Ship {//phenotype
	Brain<In, 3> brain;//genotype
	//pos, vel, rot, fov, blah, blah

	Creature() {}
};
#endif