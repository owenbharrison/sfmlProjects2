#pragma once
#ifndef Matrix_H
#define Matrix_H

template <size_t M, size_t N>
struct Matrix {
	static_assert(M>0&&N>0);
	float v[M*N]{0};

	Matrix& operator=(const Matrix& m) {
		memcpy(v, m.v, sizeof(float)*M*N);
		return *this;
	}

	inline float& operator()(size_t i, size_t j) {
		return v[i+N*j];
	}

	Matrix operator-() {
		Matrix r;
		for (size_t i=0; i<M*N; i++) r.v[i]=-v[i];
		return r;
	}

	Matrix operator+(const Matrix& m) {
		Matrix r;
		for (size_t i=0; i<M*N; i++) r.v[i]=v[i]+m.v[i];
		return r;
	}

	Matrix operator-(const Matrix& m) {
		return operator+(-m);
	}

	Matrix operator*(float f) {
		Matrix r;
		for (size_t i=0; i<M*N; i++) r.v[i]=f*v[i];
		return r;
	}
};

//by definition disallows incompatible dimensions
template <size_t M, size_t N, size_t P>
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

//flip about y=x
template<size_t M, size_t N>
Matrix<N, M> transpose(const Matrix<M, N>& in) {
	Matrix<N, M> out;
	for (size_t i=0; i<M; i++) {
		for (size_t j=0; j<N; j++) {
			out(j, i)=in(i, j);
		}
	}
	return out;
}
#endif