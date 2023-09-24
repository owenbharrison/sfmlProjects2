#pragma once
#ifndef Float2_H
#define Float2_H

namespace common {
	struct Float2 {
		float x, y;

		Float2();
		Float2(float f);
		Float2(float x, float y);

		//negation
		Float2 operator-() const;

		//addition
		Float2 operator+(const Float2 o) const;
		Float2 operator+(const float f) const;
		Float2& operator+=(const Float2 o);
		Float2& operator+=(const float f);

		//subtraction
		Float2 operator-(const Float2 o) const;
		Float2 operator-(const float f) const;
		Float2& operator-=(const Float2 o);
		Float2& operator-=(const float f);

		//multiplication
		Float2 operator*(const Float2 o) const;
		Float2 operator*(const float f) const;
		Float2& operator*=(const Float2 o);
		Float2& operator*=(const float f);

		//division
		Float2 operator/(const Float2 o) const;
		Float2 operator/(const float f) const;
		Float2& operator/=(const Float2 o);
		Float2& operator/=(const float f);
	};

	Float2 operator+(float f, const Float2 o);
	Float2 operator-(float f, const Float2 o);
	Float2 operator*(float f, const Float2 o);
	Float2 operator/(float f, const Float2 o);

	float dot(const Float2 a, const Float2 b);

	float length(const Float2 f);

	Float2 normal(const Float2 f);

	Float2 lineLineIntersection(Float2 a, Float2 b, Float2 c, Float2 d);
}
#endif