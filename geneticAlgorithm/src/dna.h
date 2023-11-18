#pragma once
#ifndef DNA_H
#define DNA_H

#include <cstdlib>
#include <cstring>
#include <string>

char randChar() {
	return ' '+rand()%95;
}

inline float rand01() {
	return float(rand())/RAND_MAX;
}

struct DNA {
	size_t num=0;
	char* genes=nullptr;
	float fitness=0;

	DNA() {}

	DNA(size_t n) : num(n) {
		genes=new char[num];
		for (size_t i=0; i<num; i++) {
			genes[i]=randChar();
		}
	}

	DNA(const DNA& d) : num(d.num) {
		genes=new char[num];
		memcpy(genes, d.genes, sizeof(char)*num);
	}

	~DNA() {
		delete[] genes;
	}

	DNA& operator=(const DNA& d) {
		if (this==&d) return *this;

		delete[] genes;

		num=d.num;
		genes=new char[num];
		memcpy(genes, d.genes, sizeof(char)*num);
		fitness=0;

		return *this;
	}

	//[0,1] (not necessary)
	void calcFitness(std::string target) {
		size_t score=0;
		for (size_t i=0; i<num; i++) {
			if (genes[i]==target[i]) score++;
		}
		fitness=float(score)/num;
	}

	DNA crossover(const DNA& o) const {
		if (num!=o.num) return *this;

		DNA baby(num);
		size_t pivot=num*rand01();
		for (size_t i=0; i<num; i++) {
			baby.genes[i]=(i<pivot?this:&o)->genes[i];
		}
		return baby;
	}

	void mutate(float rate) {
		for (size_t i=0; i<num; i++) {
			if (rand01()<rate) genes[i]=randChar();
		}
	}
};
#endif