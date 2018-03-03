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
#include "bresenham.hpp"

#include "dungeon.hpp"
#include "level.hpp"
#include "scene_manager.hpp"

bool line_of_sight(int8_t x1, int8_t y1, int8_t x2, int8_t y2, uint8_t max_dist)
{
	if (max_dist != 0 && SDL_sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)) > max_dist)
		return false;

	Dungeon *scene = engine.get_scene_manager()->get_scene("test");
	if (scene == nullptr || scene->get_level() == nullptr)
		return false;

	if (x1 == x2) // Vertical line
	{
		for (int8_t y = y1; y != y2; y += (y1 < y2 ? 1 : -1))
		{
			if (scene->get_level()->get_wall(x1, y))
				return false;
		}
		return true;
	}
	else if (y1 == y2) // Horizontal line
	{
		for (int8_t x = x1; x != x2; x += (x1 < x2 ? 1 : -1))
		{
			if (scene->get_level()->get_wall(x, y1))
				return false;
		}
		return true;
	}
	else // Curved line (here's where we actually use Bresenham's line algorithm)
	{
		// TODO - This is broken, fix it

		const float deltaerr = std::abs((y2 - y1) / (x2 - x1));
		float error = deltaerr - 0.5f;
		int8_t y = y1;

		for (int8_t x = x1; x != x2; x += (x1 < x2 ? 1 : -1))
		{
			if (scene->get_level()->get_wall(x, y))
				return false;

			error = error + deltaerr;
			if (error >= 0.5f)
			{
				y += (y1 < y2 ? 1 : -1);
				error -= 1.0f;
			}
		}
		return true;
	}
	return false;
}
