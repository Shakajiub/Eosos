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
#include "functions_ability.hpp"

#include "actor.hpp"
#include "functions_general.hpp"
#include "dungeon.hpp"
#include "scene_manager.hpp"

void register_ability_functions(lua_State *script)
{
	if (script != nullptr)
	{
		lua_register(script, "get_source_pos", lua_get_source_pos);
		lua_register(script, "set_source_state", lua_set_source_state);
		lua_register(script, "set_source_direction", lua_set_source_direction);
		lua_register(script, "get_mount_available", lua_get_mount_available);
		lua_register(script, "toggle_mount", lua_toggle_mount);
		lua_register(script, "grant_movement", lua_grant_movement);
		lua_register(script, "move_to_position", lua_move_to_position);
	}
}
int lua_get_source_pos(lua_State *script)
{
	uint8_t xpos = 0;
	uint8_t ypos = 0;

	Dungeon *scene = engine.get_scene_manager()->get_scene("test");
	if (scene != nullptr)
	{
		Actor *current_actor = scene->get_current_actor();
		if (current_actor != nullptr)
		{
			xpos = current_actor->get_grid_x();
			ypos = current_actor->get_grid_y();
		}
	}
	lua_pushnumber(script, xpos);
	lua_pushnumber(script, ypos);
	return 2;
}
int lua_set_source_state(lua_State *script)
{
	if (check_arguments(script, 1, "set_source_state"))
	{
		const uint8_t state = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 1);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr)
		{
			Actor *current_actor = scene->get_current_actor();
			//if (current_actor != nullptr)
				//current_actor->set_state((ActorState)state);
		}
	}
	return 0;
}
int lua_set_source_direction(lua_State *script)
{
	if (check_arguments(script, 1, "set_source_direction"))
	{
		const uint8_t direction = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 1);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr)
		{
			Actor *current_actor = scene->get_current_actor();
			if (current_actor != nullptr)
				current_actor->set_direction(direction);
		}
	}
	return 0;
}
int lua_get_mount_available(lua_State *script)
{
	bool mount = false;

	Dungeon *scene = engine.get_scene_manager()->get_scene("test");
	if (scene != nullptr)
	{
		Actor *current_actor = scene->get_current_actor();
		if (current_actor != nullptr)
			mount = current_actor->get_mount_available();
	}
	lua_pushboolean(script, mount);
	return 1;
}
int lua_toggle_mount(lua_State *script)
{
	Dungeon *scene = engine.get_scene_manager()->get_scene("test");
	if (scene != nullptr)
	{
		Actor *current_actor = scene->get_current_actor();
		if (current_actor != nullptr)
			current_actor->toggle_mount();
	}
	return 0;
}
int lua_grant_movement(lua_State *script)
{
	if (check_arguments(script, 1, "grant_movement"))
	{
		const uint8_t movement = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 1);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr)
		{
			Actor *current_actor = scene->get_current_actor();
			if (current_actor != nullptr)
				current_actor->grant_movement(movement);
		}
	}
	return 0;
}
int lua_move_to_position(lua_State *script)
{
	if (check_arguments(script, 2, "move_to_position"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr)
		{
			Actor *current_actor = scene->get_current_actor();
			if (current_actor != nullptr)
				current_actor->set_position(grid_x, grid_y, scene);
		}
	}
	return 0;
}
