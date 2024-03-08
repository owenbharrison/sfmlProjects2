#include "Float2.h"

namespace common {
	Float2::Float2() : Float2(0) {}

	Float2::Float2(float f) : Float2(f, f) {}

	Float2::Float2(float x, float y) : x(x), y(y) {}

	Float2 Float2::operator-() const { return {-x, -y}; }

	Float2 Float2::operator+(const Float2 o) const { return {x+o.x, y+o.y}; }
	Float2 Float2::operator+(const float f) const { return operator+(Float2(f)); }
	Float2& Float2::operator+=(const Float2 o) { *this=*this+o; return *this; }
	Float2& Float2::operator+=(const float f) { *this=*this+f; return *this; }

	Float2 Float2::operator-(const Float2 o) const { return {x-o.x, y-o.y}; }
	Float2 Float2::operator-(const float f) const { return operator-(Float2(f)); }
	Float2& Float2::operator-=(const Float2 o) { *this=*this-o; return *this; }
	Float2& Float2::operator-=(const float f) { *this=*this-f; return *this; }

	Float2 Float2::operator*(const Float2 o) const { return {x*o.x, y*o.y}; }
	Float2 Float2::operator*(const float f) const { return operator*(Float2(f)); }
	Float2& Float2::operator*=(const Float2 o) { *this=*this*o; return *this; }
	Float2& Float2::operator*=(const float f) { *this=*this*f; return *this; }

	Float2 Float2::operator/(const Float2 o) const { return {x/o.x, y/o.y}; }
	Float2 Float2::operator/(const float f) const { return operator/(Float2(f)); }
	Float2& Float2::operator/=(const Float2 o) { *this=*this/o; return *this; }
	Float2& Float2::operator/=(const float f) { *this=*this/f; return *this; }

	Float2 operator+(float f, const Float2 o) { return o+f; }
	Float2 operator-(float f, const Float2 o) { return Float2(f)-o; }
	Float2 operator*(float f, const Float2 o) { return o*f; }
	Float2 operator/(float f, const Float2 o) { return Float2(f)/o; }

	inline float dot(const Float2 a, const Float2 b) {
		Float2 c=a*b;
		return c.x+c.y;
	}

	inline float length(const Float2 f) {
		return sqrt(dot(f, f));
	}

	Float2 normal(const Float2 f) {
		float len=length(f);
		return len==0?f:f/len;
	}

	Float2 lineLineIntersection(Float2 a, Float2 b, Float2 c, Float2 d) {
		Float2 ab=a-b, ac=a-c, cd=c-d;
		float denom=ab.x*cd.y-ab.y*cd.x;
		return Float2(
			ac.x*cd.y-ac.y*cd.x,
			ac.x*ab.y-ac.y*ab.x
		)/denom;
	}
}