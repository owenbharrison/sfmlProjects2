#include "float2.h"

#include <cmath>

namespace common {
	float2::float2() : float2(0) {}

	float2::float2(float f) : float2(f, f) {}

	float2::float2(float x, float y) : x(x), y(y) {}

	float2 float2::operator-() const { return {-x, -y}; }

	float2 float2::operator+(const float2 o) const { return {x+o.x, y+o.y}; }
	float2 float2::operator+(const float f) const { return operator+(float2(f)); }
	float2& float2::operator+=(const float2 o) { *this=*this+o; return *this; }
	float2& float2::operator+=(const float f) { *this=*this+f; return *this; }

	float2 float2::operator-(const float2 o) const { return {x-o.x, y-o.y}; }
	float2 float2::operator-(const float f) const { return operator-(float2(f)); }
	float2& float2::operator-=(const float2 o) { *this=*this-o; return *this; }
	float2& float2::operator-=(const float f) { *this=*this-f; return *this; }

	float2 float2::operator*(const float2 o) const { return {x*o.x, y*o.y}; }
	float2 float2::operator*(const float f) const { return operator*(float2(f)); }
	float2& float2::operator*=(const float2 o) { *this=*this*o; return *this; }
	float2& float2::operator*=(const float f) { *this=*this*f; return *this; }

	float2 float2::operator/(const float2 o) const { return {x/o.x, y/o.y}; }
	float2 float2::operator/(const float f) const { return operator/(float2(f)); }
	float2& float2::operator/=(const float2 o) { *this=*this/o; return *this; }
	float2& float2::operator/=(const float f) { *this=*this/f; return *this; }

	float2 operator+(float f, const float2 o) { return o+f; }
	float2 operator-(float f, const float2 o) { return float2(f)-o; }
	float2 operator*(float f, const float2 o) { return o*f; }
	float2 operator/(float f, const float2 o) { return float2(f)/o; }

	float dot(const float2 a, const float2 b) {
		float2 c=a*b;
		return c.x+c.y;
	}

	float length(const float2 f) {
		return sqrt(dot(f, f));
	}

	float2 normal(const float2 f) {
		float len=length(f);
		return len==0?f:f/len;
	}

	float2 lineLineIntersection(float2 a, float2 b, float2 c, float2 d) {
		float2 ab=a-b, ac=a-c, cd=c-d;
		float denom=ab.x*cd.y-ab.y*cd.x;
		return float2(
			ac.x*cd.y-ac.y*cd.x,
			ac.x*ab.y-ac.y*ab.x
		)/denom;
	}
}