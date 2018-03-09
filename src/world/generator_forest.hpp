//	Copyright (C) 2018 Jere Oikarinen
//
//	This file is part of Eosos.
//
//	Eosos is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Eosos is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with Eosos. If not, see <http://www.gnu.org/licenses/>.

#ifndef GENERATOR_FOREST_HPP
#define GENERATOR_FOREST_HPP

#include "generator.hpp"

#include <vector>

class AStar;

enum WaveClass
{
	WAVE_NONE,
	WAVE_PEST,
	WAVE_KOBOLD,
	WAVE_DWARF,
	WAVE_DEMON
};
class GeneratorForest : public Generator
{
public:
	GeneratorForest();
	~GeneratorForest();

	virtual void free();
	virtual void init();
	virtual void render_ui();

	virtual const std::string generate(uint8_t depth);
	virtual void post_process(ActorManager *am, Level *level);
	virtual void next_turn(ActorManager *am, Level *level);

	virtual std::pair<uint8_t, uint8_t> get_base_pos() const { return base_pos; }
	virtual std::pair<uint8_t, uint8_t> get_spawn_pos() const;

private:
	void init_wave();

	uint8_t width;
	uint8_t height;
	AStar *pathfinder;

	uint8_t calm_timer;
	uint8_t spawned_mobs;
	uint8_t current_wave;
	uint8_t current_depth;
	uint16_t current_turn;

	WaveClass wave_class;
	uint8_t wave_boss;
	std::string boss_name;
	std::string boss_desc;
	std::string mount_name;
	std::vector<uint8_t> wave_monsters;
	std::pair<uint8_t, uint8_t> base_pos;
	std::vector< std::pair<uint8_t, uint8_t> > spawn_positions;
};

#endif // GENERATOR_FOREST_HPP
