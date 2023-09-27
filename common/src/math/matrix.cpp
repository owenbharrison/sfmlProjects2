#include "Matrix.h"

#define DOFORALL(x) {for(size_t i=0;i<m*n;i++)x}

namespace common {
	Matrix::Matrix() : m(0), n(0), v(nullptr) {}

	Matrix::Matrix(size_t m, size_t n) : m(m), n(n) {
		assert(m!=0, "matrix height must be nonzero");
		assert(n!=0, "matrix width must be nonzero");

		v=new float[m*n] {0};
	}

	Matrix::Matrix(size_t m, size_t n, std::vector<float> init) : Matrix(m, n) {
		assert(m*n==init.size(), "matrix init list must be of same size");

		memcpy(v, init.data(), m*n*sizeof(float));
	}

	Matrix::Matrix(const Matrix& o) : Matrix(o.m, o.n) {
		memcpy(v, o.v, m*n*sizeof(float));
	}

	Matrix::~Matrix() {
		delete[] v;
	}

	Matrix& Matrix::operator=(const Matrix& o) {
		if (this==&o) return *this;

		delete[] v;

		m=o.m, n=o.n;
		v=new float[m*n];

		memcpy(v, o.v, m*n*sizeof(float));

		return *this;
	}

	inline float& Matrix::operator()(size_t i, size_t j) const {
		return v[i*n+j];
	}

	Matrix Matrix::operator-() const {
		Matrix r(m, n);
		DOFORALL(r.v[i]=-v[i];);
		return r;
	}

	Matrix Matrix::operator+(const Matrix& o) const {
		assert(m==o.m, "matrix addition must be of same height");
		assert(n==o.n, "matrix addition must be of same width");

		Matrix r(m, n);
		DOFORALL(r.v[i]=v[i]+o.v[i];);
		return r;
	}

	Matrix& Matrix::operator+=(const Matrix& o) {
		assert(m==o.m, "matrix addition must be of same height");
		assert(n==o.n, "matrix addition must be of same width");

		DOFORALL(v[i]+=o.v[i];);
		return *this;
	}

	Matrix Matrix::operator-(const Matrix& o) const {
		return operator+(-o);
	}

	Matrix& Matrix::operator-=(const Matrix& o) {
		assert(m==o.m, "matrix addition must be of same height");
		assert(n==o.n, "matrix addition must be of same width");

		DOFORALL(v[i]-=o.v[i];);
		return *this;
	}

	Matrix Matrix::operator*(const Matrix& o) const {
		assert(n==o.m, "invalid matrix multiplication dims");

		size_t p=o.n;
		Matrix r(m, p);
		for (size_t i=0; i<m; i++) {
			for (size_t j=0; j<p; j++) {
				for (size_t k=0; k<n; k++) {
					r(i, j)+=operator()(i, k)*o(k, j);
				}
			}
		}
		return r;
	}

	Matrix Matrix::operator*(float f) const {
		Matrix r(m, n);
		DOFORALL(r.v[i]=f*v[i];);
		return r;
	}

	Matrix& Matrix::operator*=(float f) {
		DOFORALL(v[i]*=f;);
		return *this;
	}

	Matrix Matrix::transpose() const {
		Matrix r(n, m);
		for (size_t i=0; i<m; i++) {
			for (size_t j=0; j<n; j++) {
				r(j, i)=operator()(i, j);
			}
		}
		return r;
	}
}
#undef DOFORALL