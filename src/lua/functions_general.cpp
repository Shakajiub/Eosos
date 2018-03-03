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
#include "functions_general.hpp"

#include "player.hpp"
#include "dungeon.hpp"
#include "level.hpp"
#include "scene_manager.hpp"
#include "message_log.hpp"
#include "ui.hpp"

void register_general_functions(lua_State *script)
{
	if (script != nullptr)
	{
		lua_register(script, "get_base_path", lua_get_base_path);
		lua_register(script, "get_mod_state", lua_get_mod_state);
		lua_register(script, "get_player_pos", lua_get_player_pos);
		//lua_register(script, "get_node_visible", lua_get_node_visible);
		lua_register(script, "get_node_blocked", lua_get_node_blocked);
		lua_register(script, "add_log_message", lua_add_log_message);
		lua_register(script, "spawn_object", lua_spawn_object);
		lua_register(script, "damage_node", lua_damage_node);
		lua_register(script, "add_action", lua_add_action);
		lua_register(script, "finish_level", lua_finish_level);
	}
}
bool check_arguments(lua_State *script, uint8_t num, const std::string &name)
{
	// TODO - Validate arguments properly

	if (lua_gettop(script) != num)
	{
		lua_pushstring(script, (name + " called with incorrect amount of arguments!").c_str());
		lua_error(script); return false;
	}
	return true;
}
int lua_get_base_path(lua_State *script)
{
	lua_pushstring(script, engine.get_base_path().c_str());
	return 1;
}
int lua_get_mod_state(lua_State *script)
{
	bool mod_state = false;
	if (check_arguments(script, 1, "get_mod_state"))
	{
		const std::string mod = lua_tostring(script, -1);
		lua_pop(script, 1);

		if (mod == "ctrl") mod_state = (SDL_GetModState() & KMOD_CTRL);
		else if (mod == "alt") mod_state = (SDL_GetModState() & KMOD_ALT);
		else if (mod == "shift") mod_state = (SDL_GetModState() & KMOD_SHIFT);
		else std::cout << "attempting to get unknown mod state: " << mod << std::endl;
	}
	lua_pushboolean(script, mod_state);
	return 1;
}
int lua_get_player_pos(lua_State *script)
{
	lua_pushnumber(script, player->get_grid_x());
	lua_pushnumber(script, player->get_grid_y());
	return 2;
}
/*int lua_get_node_visible(lua_State *script)
{
	bool node_visible = false;
	if (check_arguments(script, 2, "get_node_visible"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
			node_visible = scene->get_level()->get_node_discovered(grid_x, grid_y);
	}
	lua_pushboolean(script, node_visible);
	return 1;
}*/
int lua_get_node_blocked(lua_State *script)
{
	bool node_blocked = true;
	if (check_arguments(script, 3, "get_node_blocked"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -3);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -2);
		const bool occupying = lua_toboolean(script, -1);
		lua_pop(script, 3);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
			node_blocked = scene->get_level()->get_wall(grid_x, grid_y, occupying);
	}
	lua_pushboolean(script, node_blocked);
	return 1;
}
int lua_add_log_message(lua_State *script)
{
	if (check_arguments(script, 2, "add_log_message"))
	{
		const std::string msg = lua_tostring(script, -2);
		const uint8_t color = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		// TODO - Get the proper color from the script
		if (ui.get_message_log() != nullptr)
			ui.get_message_log()->add_message(msg, COLOR_PEPPERMINT);
	}
	return 0;
}
int lua_spawn_object(lua_State *script)
{
	if (check_arguments(script, 4, "spawn_object"))
	{
		const int8_t xpos = lua_tointeger(script, -4);
		const int8_t ypos = lua_tointeger(script, -3);
		const std::string obj = lua_tostring(script, -2);
		const std::string type = lua_tostring(script, -1);
		lua_pop(script, 4);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
			scene->get_level()->spawn_object(xpos, ypos, obj, type);
	}
	return 0;
}
int lua_damage_node(lua_State *script)
{
	if (check_arguments(script, 4, "damage_node"))
	{
		const int8_t xpos = lua_tointeger(script, -4);
		const int8_t ypos = lua_tointeger(script, -3);
		const int8_t damage = lua_tointeger(script, -2);
		const std::string source = lua_tostring(script, -1);
		lua_pop(script, 4);

		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		if (scene != nullptr && scene->get_level() != nullptr)
			scene->get_level()->damage_node(xpos, ypos, damage, source);
	}
	return 0;
}
int lua_add_action(lua_State *script)
{
	if (check_arguments(script, 3, "add_action"))
	{
		const std::string type = lua_tostring(script, -3);
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 3);

		Actor *current_actor = nullptr;
		Dungeon *scene = engine.get_scene_manager()->get_scene("test");

		if (scene != nullptr)
			current_actor = scene->get_current_actor();

		if (current_actor != nullptr)
		{
			switch (djb_hash(type.c_str()))
			{
				case djb_hash("move"):
					current_actor->add_action(AT_MOVE, grid_x, grid_y);
					break;
				case djb_hash("attack"):
					current_actor->add_action(AT_ATTACK, grid_x, grid_y);
					break;
				default: std::cout << "attempting to add undefined action '" + type + "'!" << std::endl; break;
			}
		}
	}
	return 0;
}
int lua_finish_level(lua_State *script)
{
	Dungeon *scene = engine.get_scene_manager()->get_scene("test");
	if (scene != nullptr)
		scene->finish_level();

	return 0;
}
