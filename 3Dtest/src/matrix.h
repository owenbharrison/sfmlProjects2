#pragma once
#ifndef Matrix_H
#define Matrix_H

template <size_t M, size_t N>
struct Matrix {
	static_assert(M>0&&N>0, "Matrix dimension must be nonzero.");
	float v[M][N]{0.f};

	void operator=(const Matrix& in) {
		memcpy(this->v, in.v, sizeof(float)*M*N);
	}

	bool operator==(const Matrix& in) {
		for (size_t i=0; i<M; i++) {
			for (size_t j=0; j<N; j++) {
				if (this->v[i][j]!=in.v[i][j]) {
					return false;
				}
			}
		}
		return true;
	}

	Matrix operator+(const Matrix& in) {
		Matrix out;
		for (size_t i=0; i<M; i++) {
			for (size_t j=0; j<N; j++) {
				out.v[i][j]=this->v[i][j]+in.v[i][j];
			}
		}
		return out;
	}

	Matrix operator*(const float& in) {
		Matrix out;
		for (size_t i=0; i<M; i++) {
			for (size_t j=0; j<N; j++) {
				out.v[i][j]=this->v[i][j]*in;
			}
		}
		return out;
	}
};

//by definition disallows incompatible dimensions
template <size_t M, size_t N, size_t P>
Matrix<M, P> operator*(const Matrix<M, N>& a, const Matrix<N, P>& b) {
	Matrix<M, P> ab;
	for (size_t i=0; i<M; i++) {
		for (size_t j=0; j<P; j++) {
			float sum=0.f;
			for (size_t r=0; r<N; r++) {
				sum+=a.v[i][r]*b.v[r][j];
			}
			ab.v[i][j]=sum;
		}
	}
	return ab;
}

//flip about y=x
template<size_t M, size_t N>
Matrix<N, M> transpose(const Matrix<M, N>& in) {
	Matrix<N, M> out;
	for (size_t i=0; i<M; i++) {
		for (size_t j=0; j<N; j++) {
			out.v[j][i]=in.v[i][j];
		}
	}
	return out;
}
#endif