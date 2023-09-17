#pragma once
#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <vector>

namespace common{
	//rule of three go crazy
	struct matrix {
		size_t m, n;
		float* v;

		matrix(size_t m, size_t n);

		//init with data
		matrix(size_t m, size_t n, std::vector<float> init);
		
		//copy constructor
		matrix(const matrix& o);

		~matrix();

		//assignment overload
		matrix& operator=(const matrix& o);

		//getter
		inline float& operator()(size_t i, size_t j) const;

		//negation
		matrix operator-() const;
		
		//addition/subtraction
		matrix operator+(const matrix& o) const;
		matrix operator-(const matrix& o) const;

		//matrix multiplication
		matrix operator*(const matrix& o) const;

		//scalar multiplication
		matrix operator*(float f) const;

		//swap cols and rows
		matrix transpose() const;
	};

	//printing
	std::ostream& operator<<(std::ostream& o, const matrix& m);
}
#endif