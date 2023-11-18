#include "tetrisDemo.h"

int main() {
	auto scl=20;
	TetrisDemo demo(scl*Tetris::width, scl*Tetris::height, "Tetris AI");
	demo.setKeyRepeatEnabled(true);
	demo.setFramerateLimit(60);
	demo.run();

	return 0;
}