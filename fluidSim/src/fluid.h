#pragma once
#ifndef FLUID_H
#define FLUID_H

inline float clamp(float x, float a, float b) {
	if (x<a) return a;
	if (x>b) return b;
	return x;
}

template<typename T>
constexpr void swap(T a, T b) {
	T t=a;
	a=b, b=t;
}

struct Fluid {
	size_t width, height, size;
	float* u, * v, * dens;
	float* uPrev, * vPrev, * densPrev;

	Fluid() : width(0), height(0), size(0), u(nullptr), v(nullptr), dens(nullptr), uPrev(nullptr), vPrev(nullptr), densPrev(nullptr) {}

	Fluid(size_t w, size_t h) : width(w), height(h) {
		size=(width+2)*(height+2);

		u=new float[size];
		v=new float[size];
		dens=new float[size];
		uPrev=new float[size];
		vPrev=new float[size];
		densPrev=new float[size];
	}

	inline size_t ix(size_t i, size_t j) const {
		return i+j*(width+2);
	}

	void addSource(float* x, float* s, float dt) const {
		for (size_t i=0; i<size; i++) x[i]+=dt*s[i];
	}

	void diffuse(int b, float* x, float* x0, float diff, float dt) const {
		size_t i, j, k;
		float a=dt*diff*width*height;
		for (k=0; k<20; k++) {
			for (i=1; i<=width; i++) {
				for (j=1; j<=height; j++) {
					x[ix(i, j)]=x0[ix(i, j)]+a*(
						x[ix(i-1, j)]+
						x[ix(i+1, j)]+
						x[ix(i, j-1)]+
						x[ix(i, j+1)]
					)/(1+4*a);
				}
			}
			setBound(b, x);
		}
	}

	void advect(int b, float* d, float* d0, float* u, float* v, float dt) const {
		size_t i, j, i0, j0, i1, j1;
		float x, y, s0, t0, s1, t1, dt0;

		dt0=.5f*dt*(width+height);
		for (i=1; i<=width; i++) {
			for (j=1; j<=height; j++) {
				x=i-dt0*u[ix(i, j)];
				y=j-dt0*v[ix(i, j)];

				x=clamp(x, .5f, width+.5f); i0=x, i1=i0+1;
				y=clamp(y, .5f, height+.5f); j0=y, j1=j0+1;

				s1=x-i0, s0=1-s1;
				t1=y-j0, t0=1-t1;

				d[ix(i, j)]=
					s0*(t0*d0[ix(i0, j0)]+t1*d0[ix(i0, j1)])+
					s1*(t0*d0[ix(i1, j0)]+t1*d0[ix(i1, j1)]);
			}
		}

		setBound(b, d);
	}

	void densStep(float* x, float* x0, float* u, float* v, float diff, float dt) {
		addSource(x, x0, dt);
		swap(x0, x); diffuse(0, x, x0, diff, dt);
		swap(x0, x); advect(0, x, x0, u, v, dt);
	}

	void velStep(float* u, float* v, float* u0, float* v0, float visc, float dt) {
		addSource(u, u0, dt);
		addSource(v, v0, dt);

		swap(u0, u); swap(v0, v);
		diffuse(1, u, u0, visc, dt);
		diffuse(2, v, v0, visc, dt);

		project();

		swap(u0, u); swap(v0, v);
		advect(1, u, u0, u0, v0, dt);
		advect(2, v, v0, u0, v0, dt);

		project();
	}

	void project() {
		size_t i, j;

		float h=2.f/(width+height);
		for (i=1; i<=width; i++) {
			for (j=1; j<=height; j++) {
				vPrev[ix(i, j)]=-.5f*h*(
				u[ix(i+1, j)]-u[ix(i-1, j)]+
				v[ix(i, j+1)]-v[ix(i, j-1)]);

				uPrev[ix(i, j)]=0;
			}
		}
		setBound(0, uPrev); setBound(0, vPrev); 

		for (size_t k=0; k<20; k++) {
			for (i=1; i<=width; i++) {
				for (j=1; j<=height; j++) {
					uPrev[ix(i, j)]=vPrev[ix(i, j)]+.25f*(
					uPrev[ix(i-1, j)]+uPrev[ix(i+1, j)]+
					uPrev[ix(i, j-1)]+uPrev[ix(i, j+1)]);
				}
			}
			setBound(0, uPrev);
		}

		for (i=1; i<=width; i++) {
			for (j=1; j<=height; j++) {
				u[ix(i, j)]-=.5f*(uPrev[ix(i+1, j)]-uPrev[ix(i-1, j)])/h;
				v[ix(i, j)]-=.5f*(uPrev[ix(i, j+1)]-uPrev[ix(i, j-1)])/h;
			}
		}
		setBound(1, u); setBound(2, v);
	}

	void setBound(int b, float* x) const {
		size_t i;

		//top/bottom
		for (i=1; i<=width; i++) {
			int sgn=(b==2?-1:1);
			x[ix(i, 0)]=sgn*x[ix(i, 1)];
			x[ix(i, height+1)]=sgn*x[ix(i, height)];
		}

		//left/right
		for (i=1; i<=height; i++) {
			int sgn=(b==1?-1:1);
			x[ix(0, i)]=sgn*x[ix(1, i)];
			x[ix(width+1, i)]=sgn*x[ix(width, i)];
		}

		//corners
		x[ix(0, 0)]=.5f*(x[ix(1, 0)]+x[ix(0, 1)]);
		x[ix(0, height+1)]=.5f*(x[ix(1, height+1)]+x[ix(0, height)]);
		x[ix(width+1, 0)]=.5f*(x[ix(width, 0)]+x[ix(width+1, 1)]);
		x[ix(width+1, height+1)]=.5f*(x[ix(width, height+1)]+x[ix(width+1, height)]);
	}

	/*
	void simulate(float dt)	{
		getFromUI(uPrev, vPrev, densPrev);
		velStep(u, v, uPrev, vPrev, visc, dt);
		densStep(dens, densPrev, u, v, diff, dt);
	}	
	*/
};
#endif