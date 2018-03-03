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
#include "functions_object.hpp"

#include "dungeon.hpp"
#include "level.hpp"
#include "functions_general.hpp"
#include "object.hpp"
#include "scene_manager.hpp"

void register_object_functions(lua_State *script)
{
	if (script != nullptr)
	{
		lua_register(script, "destroy_object", lua_destroy_object);
		lua_register(script, "open_inventory", lua_open_inventory);
		lua_register(script, "close_inventory", lua_close_inventory);
		lua_register(script, "get_passable", lua_get_passable);
		lua_register(script, "get_opaque", lua_get_opaque);
	}
}
int lua_destroy_object(lua_State *script)
{
	if (check_arguments(script, 2, "destroy_object"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
			scene->get_level()->erase_object(grid_x, grid_y);
	}
	return 0;
}
int lua_open_inventory(lua_State *script)
{
	if (check_arguments(script, 2, "open_inventory"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
		{
			Object *obj = scene->get_level()->get_object(grid_x, grid_y);
			if (obj != nullptr)
				obj->toggle_inventory(1);
		}
	}
	return 0;
}
int lua_close_inventory(lua_State *script)
{
	if (check_arguments(script, 2, "close_inventory"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
		{
			Object *obj = scene->get_level()->get_object(grid_x, grid_y);
			if (obj != nullptr)
				obj->toggle_inventory(0);
		}
	}
	return 0;
}
int lua_get_passable(lua_State *script)
{
	bool obj_passable = true;
	if (check_arguments(script, 2, "get_passable"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
		{
			Object *obj = scene->get_level()->get_object(grid_x, grid_y);
			if (obj != nullptr)
				obj_passable = obj->get_passable();
		}
	}
	lua_pushboolean(script, obj_passable);
	return 1;
}
int lua_get_opaque(lua_State *script)
{
	bool obj_opaque = true;
	if (check_arguments(script, 2, "get_opaque"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
		{
			Object *obj = scene->get_level()->get_object(grid_x, grid_y);
			if (obj != nullptr)
				obj_opaque = obj->get_opaque();
		}
	}
	lua_pushboolean(script, obj_opaque);
	return 1;
}
