#pragma once
#ifndef POPULATION_H
#define POPULATION_H

#include "DNA.h"

struct Population {
	std::string target;
	size_t num=0;
	DNA* population=nullptr;
	float mutRate=0;

	Population(std::string p, size_t n, float m) : target(p), num(n), mutRate(m) {
		population=new DNA[num];
		for (size_t i=0; i<num; i++) {
			population[i]=DNA(target.length());
		}
	}

	Population(const Population& p) : target(p.target), num(p.num), mutRate(p.mutRate) {
		population=new DNA[num];
		for (size_t i=0; i<num; i++) {
			population[i]=p.population[i];
		}
	}

	~Population() {
		delete[] population;
	}

	Population& operator=(const Population& p) {
		if (this==&p) return *this;

		delete[] population;
		
		target=p.target;
		num=p.num;
		population=new DNA[num];
		for (size_t i=0; i<num; i++) {
			population[i]=p.population[i];
		}
		mutRate=p.mutRate;

		return *this;
	}

	//probability spinner(from normalized fitness distr.)	
	DNA& randomParent() const {
		float totalFitness=0;
		for (size_t i=0; i<num; i++) {
			totalFitness+=population[i].fitness;
		}
		float t=rand01();
		for (size_t i=0; i<num; i++) {
			auto& p=population[i];
			float prob=p.fitness/totalFitness;
			if (t>prob) t-=prob;
			else return p;
		}
		return population[0];
	}

	//crossover and mutate the entire pop
	void generate() {
		DNA* newPop=new DNA[num];
		for (size_t i=0; i<num; i++) {
			DNA& a=randomParent(), b=randomParent();
			newPop[i]=a.crossover(b);
			newPop[i].mutate(mutRate);
		}
		for (size_t i=0; i<num; i++) {
			population[i]=newPop[i];
		}
		delete[] newPop;
	}
};
#endif