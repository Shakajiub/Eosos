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

#ifndef FUNCTIONS_OBJECT_HPP
#define FUNCTIONS_OBJECT_HPP

// This function registers all of the functions below for a given script
void register_object_functions(lua_State *script);

// These are public incase we want to choose specifically which functions to register
int lua_destroy_object(lua_State *script);
int lua_open_inventory(lua_State *script);
int lua_close_inventory(lua_State *script);
int lua_get_passable(lua_State *script);
int lua_get_opaque(lua_State *script);

#endif // FUNCTIONS_OBJECT_HPP
