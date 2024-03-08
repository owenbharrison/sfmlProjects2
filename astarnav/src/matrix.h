#pragma once
#ifndef MATRIX_H
#define MATRIX_H

template<size_t M, size_t N>
struct Matrix {
	static_assert(M>0&&N>0, "matrix dim must be nonzero");
	float v[M*N]{0};

	Matrix& operator=(const Matrix& m) {
		memcpy(v, m.v, sizeof(float)*M*N);
	}

	inline float& operator()(size_t i, size_t j) {
		return v[i+N*j];
	}

	Matrix operator-() const {
		Matrix r;
		for (size_t i=0; i<M*N; i++) r.v[i]=-v[i];
		return r;
	}

	Matrix operator+(const Matrix& m) const {
		Matrix r;
		for (size_t i=0; i<M*N; i++) r.v[i]=v[i]+m.v[i];
		return r;
	}

	Matrix operator-(const Matrix& m) const {
		return operator+(-m);
	}

	Matrix operator*(float f) {
		Matrix r;
		for (size_t i=0; i<M*N; i++) r.v[i]=f*v[i];
		return r;
	}
};

template<size_t M, size_t N>
Matrix<N, M> transpose(const Matrix<M, N>& m) {
	Matrix<N, M> r;
	for (size_t i=0; i<M; i++) {
		for (size_t j=0; j<N; j++) {
			r(j, i)=m(j, i);
		}
	}
	return r;
}

template<size_t M, size_t N, size_t P>
Matrix<M, P> operator*(const Matrix<M, N>& a, const Matrix<N, P>& b) {
	Matrix<M, P> r;
	for (size_t i=0; i<M; i++) {
		for (size_t j=0; j<P; j++) {
			float sum=0;
			for (size_t k=0; k<N; k++) {
				sum+=a(i, k)*b(k, j);
			}
			r(i, j)=sum;
		}
	}
	return r;
}
#endif