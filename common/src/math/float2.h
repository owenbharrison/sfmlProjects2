#pragma once
#ifndef FLOAT2_H
#define FLOAT2_H

namespace common {
	struct float2 {
		float x, y;

		float2();
		float2(float f);
		float2(float x, float y);

		//negation
		float2 operator-() const;

		//addition
		float2 operator+(const float2 o) const;
		float2 operator+(const float f) const;
		float2& operator+=(const float2 o);
		float2& operator+=(const float f);

		//subtraction
		float2 operator-(const float2 o) const;
		float2 operator-(const float f) const;
		float2& operator-=(const float2 o);
		float2& operator-=(const float f);

		//multiplication
		float2 operator*(const float2 o) const;
		float2 operator*(const float f) const;
		float2& operator*=(const float2 o);
		float2& operator*=(const float f);

		//division
		float2 operator/(const float2 o) const;
		float2 operator/(const float f) const;
		float2& operator/=(const float2 o);
		float2& operator/=(const float f);
	};

	float2 operator+(float f, const float2 o);
	float2 operator-(float f, const float2 o);
	float2 operator*(float f, const float2 o);
	float2 operator/(float f, const float2 o);

	float dot(const float2 a, const float2 b);

	float length(const float2 f);

	float2 normal(const float2 f);

	float2 lineLineIntersection(float2 a, float2 b, float2 c, float2 d);
}
#endif