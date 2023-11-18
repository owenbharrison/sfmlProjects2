#pragma once
#ifndef RASTER_H
#define RASTER_H

#include <cstring>

#include <cstdlib>



namespace common {
	template<typename T>
	inline T swap(const T a, const T b) {
		T t=a;
		a=b, b=t;
	}

	struct Raster {
		size_t width=0, height=0;
		enum Colors {
			Black=0x000000FF,
			White=0xFFFFFFFF,
			Red=0xFF0000FF,
			Orange=0xFFA500FF,
			Yellow=0xFFFF00FF,
			Green=0x00FF00FF,
			Blue=0x0000FFFF,
			Cyan=0x00FFFFFF,
			Magenta=0xFF00FFFF
		};
		int* pixels=nullptr;

		Raster(size_t w, size_t h) : width(w), height(h) {
			pixels=new int[width*height] {White};
		}

		Raster(const Raster& r) : Raster(r.width, r.height) {
			memcpy(pixels, r.pixels, sizeof(int)*width*height);
		}

		~Raster() {
			delete[] pixels;
		}

		Raster& operator=(const Raster& r) {
			if (this==&r) return *this;

			delete[] pixels;

			pixels=new int[width*height];
			memcpy(pixels, r.pixels, sizeof(int)*width*height);

			return* this;
		}

		inline size_t ix(size_t i, size_t j) const {
			return pixels[i+width*j];
		}

		inline bool inRange(int x, int y) const {
			return x>=0&&x<width&&y>=0&&y<height;
		}

		void pgset(int x, int y, int col) {
			if (inRange(x, y)) pixels[ix(x, y)]=col;
		}

		//bresenhams
		void drawLine(int x1, int y1, int x2, int y2, int col=White) {
			int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
			dx=x2-x1; dy=y2-y1;
			if (dx==0) {
				if (y2<y1) swap(y1, y2);
				for (y=y1; y<=y2; y++) pgset(x1, y, col);
				return;
			}
			if (dy==0) {
				if (x2<x1) swap(x1, x2);
				for (x=x1; x<=x2; x++) pgset(x, y1, col);
				return;
			}
			dx1=abs(dx); dy1=abs(dy);
			px=2*dy1-dx1;	py=2*dx1-dy1;
			if (dy1<=dx1) {
				if (dx>=0) {
					x=x1; y=y1; xe=x2;
				} else {
					x=x2; y=y2; xe=x1;
				}
				pgset(x, y, col);
				for (i=0; x<xe; i++) {
					x++;
					if (px<0) px+=2*dy1;
					else {
						y+=((dx<0&&dy<0)||(dx>0&&dy>0))?1:-1;
						px+=2*(dy1-dx1);
					}
					pgset(x, y, col);
				}
			} else {
				if (dy>=0) {
					x=x1; y=y1; ye=y2;
				} else {
					x=x2; y=y2; ye=y1;
				}
				pgset(x, y, col);
				for (i=0; y<ye; i++) {
					y++;
					if (py<=0) py+=2*dx1;
					else {
						x+=((dx<0&&dy<0)||(dx>0&&dy>0))?1:-1;
						py+=2*(dx1-dy1);
					}
					pgset(x, y, col);
				}
			}
		}

		void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int col=White) {
			drawLine(x1, y1, x2, y2);
			drawLine(x2, y2, x3, y3);
			drawLine(x3, y3, x1, y1);
		}

		//wikipedia
		void drawCircle(int xc, int yc, int r, int col=White) {
			int x=0;
			int y=r;
			int p=3-2*r;
			if (!r) return;

			while (y>=x) {
				pgset(xc-x, yc-y, col);
				pgset(xc-y, yc-x, col);
				pgset(xc+y, yc-x, col);
				pgset(xc+x, yc-y, col);
				pgset(xc-x, yc+y, col);
				pgset(xc-y, yc+x, col);
				pgset(xc+y, yc+x, col);
				pgset(xc+x, yc+y, col);
				if (p<0) p+=4*(x++)+6;
				else p+=4*(x++-y--)+10;
			}
		}

		void drawRect(int x, int y, int w, int h, int col=White) {
			drawLine(x, y, x+w-1, y, col);
			drawLine(x+w-1, y, x+w-1, y+h-1, col);
			drawLine(x+w-1, y+h-1, x, y+h-1, col);
			drawLine(x, y+h-1, x, y, col);
		}

		const short digits[10]{
			0b0111101101101111,//0
			0b0111010010011010,//1
			0b0111001111100111,//2
			0b0111001111001111,//3
			0b0100100111101101,//4
			0b0111100111001111,//5
			0b0111101111001111,//6
			0b0010010010100111,//7
			0b0111101111101111,//8
			0b0100100111101111//9
		};
		inline bool isBitSet(short s, size_t i) {
			return (s>>i&1)==1;
		}
		
		void showDigit(int x, int y, int d, int col=White) {
			d%=10;
			for (size_t j=0; j<5; j++) {
				for (size_t i=0; i<3; i++) {
					int rx=x+i, ry=y+j;
					if (!inRange(rx, ry)) continue;
					size_t k=i+3*j;
					if (isBitSet(digits[d], k)) pgset(rx, ry, col);
				}
			}
		}

		template<typename F>
		F ddx(F f) {
			return [f] () {
				const float h=.000001f;
				return (f(x+h)-f(x))/h;
			};
		}

		template<typename F>
		void plotFunction(int x1, int y1, int x2, int y2, F func, int col=White) {
			if (x1>x2) swap(x1, x2);
			if (x1<0) x1=0;
			if (x1>=width) return;
			if (x2>=width) x2=width-1;
			for (int x=x1; x<=x2; x++) {
				int y=(func(x)-y1)/(y2-y1);
				pgset(x, y, col);
			}
		}
	};
};
#endif