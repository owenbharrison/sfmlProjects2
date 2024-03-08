#pragma once
#ifndef BLOCK_CLASS_H
#define BLOCK_CLASS_H
enum struct BlockProperty {
	None=0b00000000,
	Down=0b00000001,
	Up=0b00000010,
	Side=0b00000100,
	DownSide=0b00001000,
	UpSide=0b00010000
};
inline BlockProperty operator|(const BlockProperty a, const BlockProperty b) {
	return BlockProperty(int(a)|int(b));
}
inline auto operator&(const BlockProperty a, const BlockProperty b) {
	return int(a)&int(b);
}

enum struct BlockType {
	Air,
	Border,
	Sand,
	Dirt,
	Water,
	Lava,
	Obsidian
};

struct Block {
	BlockProperty properties=BlockProperty::None;
	BlockType type=BlockType::Air;
	float value=0;

	inline bool isFluid() const {
		return type==BlockType::Water||type==BlockType::Lava;
	}

	inline bool isDisplacable() const {
		return type==BlockType::Air||isFluid();
	}
};
#endif