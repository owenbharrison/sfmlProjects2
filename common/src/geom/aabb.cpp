#include "AABB.h"

namespace common {
	float MIN(float a, float b) { return a<b?a:b; }
	float MAX(float a, float b) { return a<b?b:a; }

	AABB::AABB() {}

	AABB::AABB(Float2 a, Float2 b) {
		min={MIN(a.x, b.x), MIN(a.y, b.y)};
		max={MAX(a.x, b.x), MAX(a.y, b.y)};
	}

	bool AABB::containsPt(const Float2 p) const {
		bool xOverlap=p.x>=min.x&&p.x<=max.x;
		bool yOverlap=p.y>=min.y&&p.y<=max.y;
		return xOverlap&&yOverlap;
	}

	bool AABB::overlaps(AABB o) const {
		bool xOverlap=min.x<=o.max.x&&max.x>=o.min.x;
		bool yOverlap=min.y<=o.max.y&&max.y>=o.min.y;
		return xOverlap&&yOverlap;
	}

	//liang barsky clipping
	//https://dos.gamebub.com/cpp_algorithms.php#lineclip
	bool AABB::clipLine(Float2& a, Float2& b) const {
		Float2 ba=b-a;
		float p, q, r, t0=0, t1=1;
		//traverse thru top, bottom, left, right
		for (int e=0; e<4; e++) {
			switch (e) {
				case 0: { p=-ba.x, q=a.x-min.x; break; }
				case 1: { p=ba.x, q=max.x-a.x; break; }
				case 2: { p=-ba.y, q=a.y-min.y; break; }
				case 3: { p=ba.y, q=max.y-a.y; break; }
			}
			r=q/p;
			if (p<0) {
				if (r>t1) return false;
				else if (r>t0) t0=r;//clip
			} else if (p>0) {
				if (r<t0) return false;
				else if (r<t1) t1=r;//clip
			} else if (q<0) return false;//parallel
		}

		//update pts
		if (t1<1) b=a+t1*ba;
		if (t0>0) a+=t0*ba;
		return true;
	}
}