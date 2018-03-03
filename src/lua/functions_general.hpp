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

#ifndef FUNCTIONS_GENERAL_HPP
#define FUNCTIONS_GENERAL_HPP

// This is used to check whether or not we have received the correct amount of arguments from lua
bool check_arguments(lua_State *script, uint8_t num, const std::string &name);

// This function registers all of the functions below for a given script
void register_general_functions(lua_State *script);

// These are public incase we want to choose specifically which functions to register
int lua_get_base_path(lua_State *script);
int lua_get_mod_state(lua_State *script);
int lua_get_player_pos(lua_State *script);
//int lua_get_node_visible(lua_State *script);
int lua_get_node_blocked(lua_State *script);
int lua_add_log_message(lua_State *script);
int lua_spawn_object(lua_State *script);
int lua_damage_node(lua_State *script);
int lua_add_action(lua_State *script);
int lua_finish_level(lua_State *script);

#endif // FUNCTIONS_GENERAL_HPP
