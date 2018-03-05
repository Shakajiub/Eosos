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

GeneratorForest::GeneratorForest()
{

}
GeneratorForest::~GeneratorForest()
{
	free();
}
void GeneratorForest::free()
{

}
void GeneratorForest::init()
{

}
const std::string GeneratorForest::generate(uint8_t depth)
{
	const int8_t offset_x[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	const int8_t offset_y[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };

	const uint8_t width = 21;
	const uint8_t height = 15;
	uint8_t map_data[width * height];

	bool map_fine = false;
	while (!map_fine)
	{
		std::fill(map_data, map_data + (width * height), 1);

		uint8_t floor_num = 1;
		uint8_t xpos = 10, ypos = 7;
		map_data[ypos * width + xpos] = 0;

		while (floor_num < 155)
		{
			if (xpos == 0 || xpos == width - 1) { map_fine = true; xpos = 10; }
			if (ypos == 0 || ypos == height - 1) { map_fine = true; ypos = 7; }

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
			std::cout << "map discarded, no access to edge ..." << std::endl;
	}
	/*for (uint8_t y = 0; y < height; y++)
	{
		for (uint8_t x = 0; x < width; x++)
		{

		}
	}*/
	std::string level =
		"l-0-core/texture/level/floor/dark2_base.png\n"
		"l-1-core/texture/level/floor/dark2_grass.png\n"
		"l-T-core/texture/level/tree/oak_dead.png\n";
	for (uint8_t y = 0; y < height; y++)
	{
		for (uint8_t x = 0; x < width; x++)
		{
			const uint8_t node = map_data[y * width + x];
			level += (engine.get_rng() % 2 == 0) ? '0' : '1';
			if (node == 0)
				level += '_';
			else level += 'T';
		}
		level += '\n';
	}
	std::cout << level << std::endl;
	return level;
}
void GeneratorForest::next_turn(uint16_t turn, ActorManager *am)
{
	std::cout << "forest generator turn #" << (int)turn << std::endl;
}
