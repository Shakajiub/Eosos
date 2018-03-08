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

#include "mount.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"

Monster::Monster() : pathfinder(nullptr), healthbar(nullptr)
{
	health = std::make_pair(2, 2);
	name = "Dwarf";
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
	if (healthbar != nullptr)
	{
		engine.get_texture_manager()->free_texture(healthbar->get_name());
		healthbar = nullptr;
	}
}
bool Monster::init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name)
{
	if (!Actor::init(at, xpos, ypos, texture_name))
		return false;

	return (init_pathfinder() && init_healthbar());
}
void Monster::render() const
{
	Actor::render();

	if (healthbar != nullptr && in_camera && health.first > 0 &&
		((hovered != HOVER_NONE) || health.first < health.second))
	{
		const uint8_t hp_percent = (float)health.first / (float)health.second * 14;
		const SDL_Rect temp_rect = { 28 - (hp_percent * 2), 0, 3, 16 };

		healthbar->render(
			x - camera.get_cam_x() + (facing_right ? 0 : 26),
			y - camera.get_cam_y(),
			&temp_rect, 2, SDL_FLIP_NONE, 0.0
		);
	}
}
void Monster::start_turn()
{
	turn_done = false;

	const uint8_t temp_moves = (mount != nullptr) ? 2 : 1;
	moves = std::make_pair(temp_moves, temp_moves);

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
bool Monster::init_healthbar()
{
	healthbar = engine.get_texture_manager()->load_texture("core/texture/ui/health_bar.png");
	return healthbar != nullptr;
}
bool Monster::init_pathfinder()
{
	pathfinder = new AStar;
	return pathfinder->init();
}
void Monster::step_pathfinder(Level *level)
{
	Actor *temp_actor = level->get_actor(pathfinder->get_goto_x(), pathfinder->get_goto_y());
	if (temp_actor != nullptr)
	{
		if (temp_actor->get_actor_type() == ACTOR_HERO)
			add_action(ACTION_ATTACK, pathfinder->get_goto_x(), pathfinder->get_goto_y());

		else if (temp_actor->get_actor_type() == ACTOR_MOUNT)
		{
			add_action(ACTION_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());
			set_mount(dynamic_cast<Mount*>(temp_actor));
		}
		else turn_done = true;
		moves.first = 0;
	}
	else
	{
		moves.first -= 1;
		add_action(ACTION_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());
	}
}
