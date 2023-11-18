#pragma once
#ifndef POPULATION_H
#define POPULATION_H

#include "creature.h"

struct Population {
	size_t num=0;
	std::vector<Creature> creatures;
	float mutRate=0;

	Population(size_t n, float m) : num(n), mutRate(m) {}
};
#endif