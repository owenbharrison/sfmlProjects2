#pragma once
#ifndef MATRix_H
#define MATRix_H

#include <cassert>
#include <vector>
#include <functional>

namespace common{
	typedef std::function<float(float)> UnaryFloatFunc;

	//rule of three go crazy
	struct Matrix {
		size_t m, n;
		float* v;

		Matrix();

		Matrix(size_t m, size_t n);

		//init with data
		Matrix(size_t m, size_t n, std::vector<float> init);
		
		//copy constructor
		Matrix(const Matrix& o);

		~Matrix();

		//assignment overload
		Matrix& operator=(const Matrix& o);

		//getter
		inline float& operator()(size_t i, size_t j) const;

		//setter
		Matrix forEach(UnaryFloatFunc func);

		//negation
		Matrix operator-() const;
		
		//addition/subtraction
		Matrix operator+(const Matrix& o) const;
		Matrix& operator+=(const Matrix& o);
		Matrix operator-(const Matrix& o) const;
		Matrix& operator-=(const Matrix& o);

		//Matrix multiplication
		Matrix operator*(const Matrix& o) const;

		//scalar multiplication
		Matrix operator*(float f) const;
		Matrix& operator*=(float f);

		//swap cols and rows
		Matrix transpose() const;
	};
}
#endif