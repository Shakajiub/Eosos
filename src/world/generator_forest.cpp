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

#include "engine.hpp"
#include "generator_forest.hpp"
#include "actor_manager.hpp"
#include "level.hpp"
#include "astar.hpp"

#include <unordered_map>

GeneratorForest::GeneratorForest() : pathfinder(nullptr)
{

}
GeneratorForest::~GeneratorForest()
{
	free();
}
void GeneratorForest::free()
{
	if (pathfinder != nullptr)
	{
		delete pathfinder;
		pathfinder = nullptr;
	}
	spawn_positions.clear();
}
void GeneratorForest::init()
{
	pathfinder = new AStar;
	pathfinder->init();
}
const std::string GeneratorForest::generate(uint8_t depth)
{
	const int8_t offset_x[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	const int8_t offset_y[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };

	width = 25; height = 15;
	uint8_t map_data[width * height];
	base_pos = std::make_pair(5, 7);
	base_pos.second += (engine.get_rng() % 5) - 2;

	bool map_fine = false;
	while (!map_fine)
	{
		spawn_positions.clear();
		std::fill(map_data, map_data + (width * height), 1);

		uint8_t floor_num = 1;
		uint8_t xpos = base_pos.first;
		uint8_t ypos = base_pos.second;
		map_data[ypos * width + xpos] = 0;

		while (floor_num < 180)
		{
			if (xpos == width - 1)
			{
				spawn_positions.push_back(std::make_pair(xpos, ypos));
				xpos = base_pos.first;
				ypos = base_pos.second;
				map_fine = true;
			}
			else if (xpos == 0 || ypos == 0 || ypos == height - 1)
			{
				xpos = base_pos.first;
				ypos = base_pos.second;
			}
			const uint8_t dir = engine.get_rng() % 4;
			xpos += offset_x[dir];
			ypos += offset_y[dir];

			if (map_data[ypos * width + xpos] == 1)
			{
				map_data[ypos * width + xpos] = 0;
				floor_num += 1;
			}
		}
		if (!map_fine)
			std::cout << "map discarded, no access to right edge" << std::endl;
	}
	std::string level =
		"l-0-core/texture/level/floor/dark2_base.png\n"
		"l-1-core/texture/level/floor/dark2_grass.png\n"
		"l-2-core/texture/level/floor/dark2_hill.png\n"
		"l-T-core/texture/level/tree/oak_dead.png\n"
		"l-M-core/texture/level/hill/dark_blue.png\n"
		"l-#-core/texture/level/map/road_dark2.png\n"
		"l-B-core/texture/level/map/base_outpost.png\n";
	for (uint8_t y = 0; y < height; y++)
	{
		for (uint8_t x = 0; x < width; x++)
		{
			const uint8_t node = map_data[y * width + x];
			if (x < 15)
				level += (engine.get_rng() % 2 == 0) ? '0' : '1';
			else level += (engine.get_rng() % 4 == 0) ? '1' : '2';
			if (node == 1)
			{
				if (engine.get_rng() % ((x + 1) * 4) > 20)
					level += 'M';
				else level += 'T';
			}
			else level += '_';
		}
		level += '\n';
	}
	std::cout << level << std::endl;
	return level;
}
void GeneratorForest::post_process(uint8_t depth, Level *level)
{
	if (pathfinder == nullptr)
	{
		pathfinder = new AStar;
		pathfinder->init();
	}
	else pathfinder->clear_path();

	const std::pair<uint8_t, uint8_t> start = spawn_positions[engine.get_rng() % spawn_positions.size()];
	const uint8_t start_x = start.first;
	const uint8_t start_y = start.second;

	spawn_positions.clear();
	spawn_positions.push_back(start);

	auto sub_nodes = level->get_sub_nodes();
	MapNode new_node = { nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false };
	MapNode base_node = { nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false };

	new_node.floor_texture = sub_nodes['0'].sub_texture;
	new_node.wall_texture = sub_nodes['#'].sub_texture;
	new_node.wall_animated = sub_nodes['#'].sub_animated;
	new_node.wall_type = sub_nodes['#'].sub_type;

	base_node.floor_texture = sub_nodes['0'].sub_texture;
	base_node.wall_texture = sub_nodes['B'].sub_texture;
	base_node.wall_animated = sub_nodes['B'].sub_animated;
	base_node.wall_type = sub_nodes['B'].sub_type;

	pathfinder->find_path(level, start_x, start_y, base_pos.first, base_pos.second, false);
	while (pathfinder->get_path_found())
	{
		level->set_node(pathfinder->get_goto_x(), pathfinder->get_goto_y(), new_node);
		pathfinder->step();
	}
	level->set_node(start_x, start_y, new_node);
	level->set_node(base_pos.first, base_pos.second, base_node);
}
void GeneratorForest::next_turn(uint16_t turn, ActorManager *am, Level *level)
{
	std::cout << "forest generator turn #" << (int)turn << std::endl;
	if (am != nullptr && turn % 2 == 0)
	{
		auto pos = get_spawn_pos();
		am->spawn_actor(level, ACTOR_MONSTER, pos.first, pos.second, "core/texture/actor/player/dwarf/dwarf_7.png");
	}
}
std::pair<uint8_t, uint8_t> GeneratorForest::get_spawn_pos() const
{
	if (spawn_positions.size() == 0)
		return std::make_pair(0, 0);
	return spawn_positions[engine.get_rng() % spawn_positions.size()];
}
