#pragma once
#ifndef TETRIS_H
#define TETRIS_H

#include <cstring>

#include <random>
std::mt19937_64 generator(time(NULL));
int random(int n) {
	std::uniform_int_distribution<int> dist(0, n);
	return dist(generator)%n;
}

inline size_t negMod(int a, size_t b) {
	return (a%b+b)%b;
}

struct Tetris {
	static const size_t width=10, height=22;
	bool* grid=nullptr;
	const bool tetros[7][9]{
		0,1,0, 0,1,0, 1,1,0,//L
		0,1,0, 0,1,0, 0,1,1,//J
		1,0,0, 1,1,0, 0,1,0,//S
		0,0,1, 0,1,1, 0,1,0,//Z
		0,1,0, 1,1,1, 0,0,0,//T
		0,1,0, 0,1,0, 0,1,0,//|
		1,1,0, 1,1,0, 0,0,0//sqr
	};

	size_t curTet;
	int curX, curY, curRot;

	float dropTimer=0;
	float dropTime=1.f;

	size_t level=0;
	size_t score=0;
	size_t lines=0;

	Tetris() {
		grid=new bool[width*height] {0};
		
		randPiece();
	}

	Tetris(const Tetris& t) : curTet(t.curTet), curX(t.curX), curY(t.curY), curRot(t.curRot), dropTimer(t.dropTimer), dropTime(t.dropTime) {
		grid=new bool[width*height];
		memcpy(grid, t.grid, sizeof(bool)*width*height);
	}

	~Tetris() {
		delete[] grid;
	}

	Tetris& operator=(const Tetris& t) {
		if(this==&t) return *this;

		memcpy(grid, t.grid, sizeof(bool)*width*height);

		curTet=t.curTet;
		curX=t.curX, curY=t.curY;
		curRot=t.curRot;

		dropTimer=t.dropTimer;
		dropTime=t.dropTime;

		return *this;
	}

	inline void randPiece() {
		curTet=random(7);
		curX=(width-3)/2, curY=0;
		curRot=random(4);
	}

	inline bool inRange(int i, int j) const {
		return i>=0&&i<width&&j>=0&&j<height;
	}
	inline size_t gIx(size_t i, size_t j) const {
		return i+width*j;
	}
	size_t tIx(size_t i, size_t j, int r) const {
		switch (negMod(r, 4)) {
			case 0: return i+3*j;
			case 1: return 6-3*i+j;
			case 2: return 8-i-3*j;
			case 3: return 2+3*i-j;
		}
		return 0;
	}

	bool canFit(size_t t, int x, int y, int r) const {
		for (size_t i=0; i<3; i++) {
			for (size_t j=0; j<3; j++) {
				if (!tetros[t][tIx(i, j, r)]) continue;

				int rx=x+i, ry=y+j;
				//hits wall
				if (!inRange(rx, ry)) return false;

				//hits grid
				if (grid[gIx(rx, ry)]) return false;
			}
		}
		return true;
	}

	inline void moveLeft() { canFit(curTet, curX-1, curY, curRot)&&curX--; }
	inline void moveRight() { canFit(curTet, curX+1, curY, curRot)&&curX++; }
	inline void rotateLeft() { canFit(curTet, curX, curY, curRot-1)&&curRot--; }
	inline void rotateRight() { canFit(curTet, curX, curY, curRot+1)&&curRot++; }

	//whether it could drop...
	bool softDrop(bool toScore=false) {
		if (canFit(curTet, curX, curY+1, curRot)) {
			curY++;
			if (toScore) score++;
			return true;
		}

		//set in stone
		for (size_t i=0; i<3; i++) {
			for (size_t j=0; j<3; j++) {
				if (tetros[curTet][tIx(i, j, curRot)]) {
					grid[gIx(curX+i, curY+j)]=true;
				}
			}
		}

		randPiece();

		return false;
	}

	inline void hardDrop() { while (softDrop(true)) score++; }

	void checkClear() {
		size_t num=0;
		for (size_t j=0; j<height; j++) {
			bool clear=true;
			for (size_t i=0; i<width; i++) {
				if (!grid[gIx(i, j)]) {
					clear=false;
					break;
				}
			}
			if (!clear) continue;
			
			//shift upper rows down, clear top row
			memmove(grid+width, grid, sizeof(bool)*width*j);
			memset(grid, false, sizeof(bool)*width);
			num++;
		}

		lines+=num;
		level=lines/5+1;

		size_t amt=0;
		switch (num) {
			case 1: amt=50; break;
			case 2: amt=100; break;
			case 3: amt=250; break;
		}
		score+=amt*level;
	}

	void update(float dt) {
		if (dropTimer<0) {
			dropTimer=dropTime;

			softDrop();
		}
		dropTimer-=dt;

		checkClear();
	}
};
#endif