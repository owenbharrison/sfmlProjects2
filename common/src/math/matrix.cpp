#include "matrix.h"

#define ASSERT(b, m) {if(!(b)) { std::cerr<<"Error: "<<(m); exit(-1); }}

namespace common {
	matrix::matrix(size_t m=1, size_t n=1) : m(m), n(n) {
		ASSERT(m>0, "matrix height must be nonzero");
		ASSERT(n>0, "matrix width must be nonzero");

		v=new float[m*n] {0};
	}

	matrix::matrix(size_t m, size_t n, std::vector<float> init) : matrix(m, n) {
		ASSERT(m*n==init.size(), "matrix init list must be of same size");

		memcpy(v, init.data(), m*n*sizeof(float));
	}

	matrix::matrix(const matrix& o) : matrix(o.m, o.n) {
		memcpy(v, o.v, m*n*sizeof(float));
	}

	matrix::~matrix() {
		delete[] v;
	}

	matrix& matrix::operator=(const matrix& o) {
		if (this==&o) return *this;

		delete[] v;

		m=o.m, n=o.n;
		v=new float[m*n];

		memcpy(v, o.v, m*n*sizeof(float));

		return *this;
	}

	inline float& matrix::operator()(size_t i, size_t j) const {
		return v[i*n+j];
	}

	matrix matrix::operator-() const {
		matrix r(m, n);
		for (size_t i=0; i<m*n; i++) r.v[i]=-v[i];
		return r;
	}

	matrix matrix::operator+(const matrix& o) const {
		ASSERT(m==o.m, "matrix addition must be of same height");
		ASSERT(n==o.n, "matrix addition must be of same width");

		matrix r(m, n);
		for (size_t i=0; i<m*n; i++) r.v[i]=v[i]+o.v[i];
		return r;
	}

	matrix matrix::operator-(const matrix& o) const {
		return operator+(-o);
	}

	matrix matrix::operator*(const matrix& o) const {
		ASSERT(n==o.m, "invalid matrix multiplication dims");

		size_t p=o.n;
		matrix r(m, p);
		for (size_t i=0; i<m; i++) {
			for (size_t j=0; j<p; j++) {
				for (size_t k=0; k<n; k++) {
					r(i, j)+=operator()(i, k)*o(k, j);
				}
			}
		}
		return r;
	}

	matrix matrix::operator*(float f) const {
		matrix r(m, n);
		for (size_t i=0; i<m*n; i++) r.v[i]=f*v[i];
		return r;
	}

	matrix matrix::transpose() const {
		matrix r(n, m);
		for (size_t i=0; i<m; i++) {
			for (size_t j=0; j<n; j++) {
				r(j, i)=operator()(i, j);
			}
		}
		return r;
	}

	std::ostream& operator<<(std::ostream& o, const matrix& m) {
		o<<'('<<m.m<<'x'<<m.n<<")\n";
		for (size_t i=0; i<m.m; i++) {
			for (size_t j=0; j<m.n; j++) o<<m(i, j)<<' ';
			o<<'\n';
		}
		return o;
	}
}