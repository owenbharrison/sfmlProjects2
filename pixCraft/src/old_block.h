#pragma once
#ifndef BLOCK_H
#define BLOCK_H

enum BlockType {
	Air,
	Border,
	Dirt,
	Sand,
	Water,
	Lava,
	Obsidian
};

struct Block {
	enum Type {
		Air,
		Border,
		Dirt,
		Sand,
		Water,
		Lava,
		Obsidian
	} type=Air;
	float val=0;

	inline bool isFluid() const {
		return type==Water||type==Lava;
	}

	//"can something sink thrrough this?"
	inline bool isDisplacable() const {
		return type==Air||isFluid();
	}
};
#endif