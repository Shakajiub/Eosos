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

#ifndef FUNCTIONS_ABILITY_HPP
#define FUNCTIONS_ABILITY_HPP

// This function registers all of the functions below for a given script
void register_ability_functions(lua_State *script);

// These are public incase we want to choose specifically which functions to register
int lua_get_source_pos(lua_State *script);
int lua_set_source_state(lua_State *script);
int lua_set_source_direction(lua_State *script);
int lua_get_mount_available(lua_State *script);
int lua_toggle_mount(lua_State *script);
int lua_grant_movement(lua_State *script);
int lua_move_to_position(lua_State *script);

#endif // FUNCTIONS_ABILITY_HPP
