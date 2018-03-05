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
#include "hero.hpp"
#include "level.hpp"
#include "astar.hpp"

#include "camera.hpp"

Hero::Hero() : auto_move_path(false), command_this_turn(false), pathfinder(nullptr)
{

}
Hero::~Hero()
{
	free();
}
void Hero::free()
{
	if (pathfinder != nullptr)
	{
		delete pathfinder;
		pathfinder = nullptr;
	}
}
void Hero::render() const
{
	Actor::render();

	if (pathfinder != nullptr && hovered)
		pathfinder->render(moves.first);
}
void Hero::start_turn()
{
	turn_done = false;
	moves = std::make_pair(2, 2);
}
bool Hero::take_turn(Level *level)
{
	hovered = true;
	if (turn_done)
	{
		if (pathfinder != nullptr && pathfinder->get_path_found())
		{
			if (grid_x == pathfinder->get_goto_x() && grid_y == pathfinder->get_goto_y())
				pathfinder->step();
			else pathfinder->clear_path();
		}
		command_this_turn = false;
		if (moves.first > 0)
			turn_done = false;
	}
	if (actions_empty() && moves.first > 0)
	{
		if (auto_move_path && pathfinder != nullptr)
		{
			step_pathfinder(level);
			if (!pathfinder->get_path_found())
				auto_move_path = false;
		}
	}
	return Actor::take_turn(level);
}
void Hero::end_turn()
{
	Actor::end_turn();
	hovered = false;
}
void Hero::init_pathfinder()
{
	pathfinder = new AStar;
	pathfinder->init();
}
void Hero::step_pathfinder(Level *level)
{
	const Actor *temp_actor = level->get_actor(pathfinder->get_goto_x(), pathfinder->get_goto_y());
	if (temp_actor != nullptr)
	{
		if (grid_x == pathfinder->get_goto_x() && grid_y == pathfinder->get_goto_y())
			return;

		moves.first = 0;
		add_action(ACTION_ATTACK, pathfinder->get_goto_x(), pathfinder->get_goto_y());
		pathfinder->clear_path();
	}
	else
	{
		moves.first -= 1;
		add_action(ACTION_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());

		if (command_this_turn && !get_auto_move())
			camera.update_position(pathfinder->get_goto_x() * 32, pathfinder->get_goto_y() * 32);
	}
}
void Hero::input_keyboard_down(SDL_Keycode key, Level *level)
{
	if (!actions_empty() || moves.first <= 0)
		return;

	int8_t offset_x = 0, offset_y = 0;
	switch (key)
	{
		case SDLK_UP: case SDLK_KP_8: case SDLK_k: offset_y = -1; break;
		case SDLK_DOWN: case SDLK_KP_2: case SDLK_j: offset_y = 1; break;
		case SDLK_LEFT: case SDLK_KP_4: case SDLK_h: offset_x = -1; break;
		case SDLK_RIGHT: case SDLK_KP_6: case SDLK_l: offset_x = 1; break;
		case SDLK_KP_7: case SDLK_y: offset_x = -1; offset_y = -1; break;
		case SDLK_KP_9: case SDLK_u: offset_x = 1; offset_y = -1; break;
		case SDLK_KP_1: case SDLK_b: offset_x = -1; offset_y = 1; break;
		case SDLK_KP_3: case SDLK_n: offset_x = 1; offset_y = 1; break;
		case SDLK_KP_5: case SDLK_SPACE: turn_done = true; moves.first = 0; break;
		default: break;
	}
	if (offset_x != 0 || offset_y != 0)
	{
		Actor *temp_actor = level->get_actor(grid_x + offset_x, grid_y + offset_y);
		if (temp_actor != nullptr && temp_actor->get_actor_type() == ACTOR_MONSTER)
		{
			moves.first = 0;
			add_action(ACTION_ATTACK, grid_x + offset_x, grid_y + offset_y);
		}
		else if (!level->get_wall(grid_x + offset_x, grid_y + offset_y, true))
		{
			moves.first -= 1;
			add_action(ACTION_MOVE, grid_x + offset_x, grid_y + offset_y);
			camera.update_position((grid_x + offset_x) * 32, (grid_y + offset_y) * 32);
		}
	}
}
void Hero::input_mouse_button_down(SDL_Event eve, Level *level)
{
	if (pathfinder != nullptr)
	{
		const int16_t map_x = (eve.button.x + camera.get_cam_x()) / 32;
		const int16_t map_y = (eve.button.y + camera.get_cam_y()) / 32;

		if (map_x == grid_x && map_y == grid_y)
		{
			if (auto_move_path)
				auto_move_path = false;
			else turn_done = true;
			return;
		}
		if (pathfinder->get_path_found())
		{
			// If we don't click the end of the path, recalculate it (if we're not moving)
			if (map_x != pathfinder->get_last_x() || map_y != pathfinder->get_last_y())
			{
				pathfinder->clear_path();
				if (!auto_move_path)
					pathfinder->find_path(level, grid_x, grid_y, (int8_t)map_x, (int8_t)map_y);
				auto_move_path = false;
			}
			else // If we click the end of a path, start moving there automatically
			{
				command_this_turn = true;
				auto_move_path = true;
			}
		}
		// Otherwise just calculate the new path
		else pathfinder->find_path(level, grid_x, grid_y, (int8_t)map_x, (int8_t)map_y);
	}
}
bool Hero::get_auto_move() const
{
	if (auto_move_path)
	{
		if (pathfinder->get_path_found())
			return pathfinder->get_length() > 1;
	}
	return false;
}
