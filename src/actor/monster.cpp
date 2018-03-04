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
#include "monster.hpp"
#include "level.hpp"

Monster::Monster()
{

}
Monster::~Monster()
{

}
void Monster::start_turn()
{
	turn_done = false;
	moves = std::make_pair(1, 1);
}
bool Monster::take_turn(Level *level)
{
	if (action_queue.empty() && moves.first > 0)
	{
		const int8_t offset_x = -1;
		const int8_t offset_y = (engine.get_rng() % 3) - 1;

		if (!level->get_wall(grid_x + offset_x, grid_y + offset_y, true))
		{
			moves.first -= 1;
			add_action(ACTION_MOVE, grid_x + offset_x, grid_y + offset_y);
		}
	}
	if (turn_done)
	{
		if (moves.first > 0)
			turn_done = false;
	}
	return Actor::take_turn(level);
}
void Monster::end_turn()
{
	Actor::end_turn();
}
