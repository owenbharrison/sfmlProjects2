#include <iostream>

#include <cstring>

#include <random>
static std::mt19937_64 generator(time(NULL));
static inline float rand01() {
	static std::uniform_real_distribution<float> dist(0, 1);
	return dist(generator);
}

static inline size_t randN(size_t n) {
	static std::uniform_int_distribution<size_t> dist(0, n-1);
	return dist(generator);
}

struct Domino {
	int a=0, b=0;
	bool used=false;
};

int main() {
	//init possible doms
	Domino possible[28];
	for (int i=0, k=0; i<=6; i++) {
		for (int j=i; j<=6; j++) {
			possible[k++]={i, j};
		}
	}

	//init grid
	int grid[8][7];
	memset(grid, -1, sizeof(int)*56);

	auto isValid=[&grid] (int i, int j) {
		if (i<0||j<0||i>7||j>6) return false;
		return grid[i][j]==-1;
	};

	auto printGrid=[&grid] () {
		for (size_t j=0; j<7; j++) {
			for (size_t i=0; i<8; i++) {
				const int& v=grid[i][j];
				if (v==-1) std::cout<<"  ";
				else std::cout<<v<<' ';
			}
			std::cout<<std::endl;
		}
	};

	//spiral loop
	int di=1, dj=0;
	size_t top=1, rgt=7, btm=6, lft=0;
	for (size_t n=0, i=0, j=0; n<56; n++) {
		int& curr=grid[i][j];

		//try add dom
		if (curr==-1) {
			bool front=isValid(i+di, j+dj);
			bool right=isValid(i-dj, j+di);
			bool diag=isValid(i+di-dj, j+dj+di);

			int type=-1;
			switch (front+2*diag+4*right) {
				case 7: type=rand01()<.5f; break;//either
				case 6: case 4: type=1; break;//right
				case 5: case 3: case 1: type=0; break;//front
			}
			if (type!=-1) {
				//random domino
				size_t d;
				do d=randN(28);
				while (possible[d].used);
				possible[d].used=true;

				//random flippage
				int a=possible[d].a, b=possible[d].b;
				if (rand01()<.5f) {//swap
					int t=a;
					a=b, b=t;
				}

				//place the jawn
				curr=a;
				if (type) grid[i-dj][j+di]=b;
				else grid[i+di][j+dj]=b;

				std::cout<<"placed #"<<d<<std::endl;
				printGrid();
			}
		}

		//check bounds and change dir
		bool rot=false;
		if (di==1&&i==rgt) rgt--, rot=true;
		else if (dj==1&&j==btm) btm--, rot=true;
		else if (di==-1&&i==lft) lft++, rot=true;
		else if (dj==-1&&j==top) top++, rot=true;
		if (rot) {
			int t=di;
			di=-dj, dj=t;
		}

		//step
		i+=di;
		j+=dj;
	}

	return 0;
}