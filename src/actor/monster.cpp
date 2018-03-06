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
#include "astar.hpp"

Monster::Monster() : pathfinder(nullptr)
{

}
Monster::~Monster()
{
	free();
}
void Monster::free()
{
	if (pathfinder != nullptr)
	{
		delete pathfinder;
		pathfinder = nullptr;
	}
}
void Monster::start_turn()
{
	turn_done = false;
	moves = std::make_pair(1, 1);

	if (pathfinder != nullptr)
		pathfinder->clear_path();
}
bool Monster::take_turn(Level *level)
{
	if (turn_done)
	{
		if (pathfinder != nullptr && pathfinder->get_path_found())
		{
			if (grid_x == pathfinder->get_goto_x() && grid_y == pathfinder->get_goto_y())
				pathfinder->step();
		}
		if (moves.first > 0)
			turn_done = false;
	}
	if (actions_empty() && moves.first > 0)
	{
		if (pathfinder != nullptr)
		{
			if (level->get_wall_type(grid_x, grid_y) == NT_BASE)
			{
				delete_me = true;
				return true;
			}
			if (!pathfinder->get_path_found())
			{
				auto base_pos = level->get_base_pos();
				pathfinder->find_path(level, grid_x, grid_y, base_pos.first, base_pos.second);
			}
			step_pathfinder(level);
		}
	}
	return Actor::take_turn(level);
}
void Monster::end_turn()
{
	Actor::end_turn();
}
void Monster::init_pathfinder()
{
	pathfinder = new AStar;
	pathfinder->init();
}
void Monster::step_pathfinder(Level *level)
{
	const Actor *temp_actor = level->get_actor(pathfinder->get_goto_x(), pathfinder->get_goto_y());
	if (temp_actor != nullptr)
	{
		moves.first = 0;
		if (temp_actor->get_actor_type() != actor_type)
			add_action(ACTION_ATTACK, pathfinder->get_goto_x(), pathfinder->get_goto_y());
	}
	else
	{
		moves.first -= 1;
		add_action(ACTION_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());
	}
}
