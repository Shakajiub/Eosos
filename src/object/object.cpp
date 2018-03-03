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
#include "object.hpp"
#include "item.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "functions_general.hpp"
#include "functions_object.hpp"
#include "lua_manager.hpp"
#include "texture_manager.hpp"
#include "inventory.hpp"
#include "ui.hpp"

Object::Object() :
	passable(false), opaque(false), animated(false), updated(false), delete_flag(false),
	obj_script(nullptr), obj_texture(nullptr)
{

}
Object::~Object()
{
	free();
}
void Object::free()
{
	if (obj_script != nullptr)
	{
		engine.get_lua_manager()->free_script(obj_script_name);
		obj_script = nullptr;
	}
	if (obj_texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(obj_texture->get_name());
		obj_texture = nullptr;
	}
	for (uint8_t i = 0; i < inventory.capacity(); i++)
	{
		if (inventory[i] != nullptr)
			delete inventory[i];
	}
	inventory.clear();
}
void Object::render() const
{
	if (obj_texture != nullptr && camera.get_in_camera_grid(grid_x, grid_y))
		obj_texture->render((grid_x * 32) - camera.get_cam_x(), (grid_y * 32) - camera.get_cam_y(), &frame_rect);
}
bool Object::construct(uint8_t xpos, uint8_t ypos, const std::string &script, const std::string &type)
{
	obj_script = engine.get_lua_manager()->load_script(script);
	if (obj_script != nullptr)
	{
		grid_x = xpos; grid_y = ypos;
		obj_script_name = script;
		frame_rect = { 0, 0, 16, 16 };

		if (engine.get_lua_manager()->get_reference_count(script) < 2)
		{
			register_general_functions(obj_script);
			register_object_functions(obj_script);
		}
		lua_getglobal(obj_script, "construct");
		lua_pushnumber(obj_script, grid_x);
		lua_pushnumber(obj_script, grid_y);
		lua_pushstring(obj_script, type.c_str());

		if (lua_pcall(obj_script, 3, 7, 0) == LUA_OK)
		{
			frame_rect.x = (int)lua_tointeger(obj_script, -7);
			frame_rect.y = (int)lua_tointeger(obj_script, -6);
			const std::string texture = lua_tostring(obj_script, -5);
			passable = lua_toboolean(obj_script, -4);
			opaque = lua_toboolean(obj_script, -3);
			animated = lua_toboolean(obj_script, -2);
			updated = lua_toboolean(obj_script, -1);
			lua_pop(obj_script, 7);

			obj_texture = engine.get_texture_manager()->load_texture(texture);
			if (obj_texture != nullptr)
			{
				inventory.clear();
				inventory.reserve(15);

				for (uint8_t i = 0; i < 15; i++)
					inventory[i] = nullptr;

				return true;
			}
		}
		else std::cout << "lua error: " << lua_tostring(obj_script, -1) << std::endl;
	}
	return false;
}
void Object::interact()
{
	if (obj_script != nullptr)
	{
		lua_getglobal(obj_script, "interact");
		lua_pushnumber(obj_script, grid_x);
		lua_pushnumber(obj_script, grid_y);
		lua_pushnumber(obj_script, frame_rect.x);

		if (lua_pcall(obj_script, 3, 4, 0) == LUA_OK)
		{
			frame_rect.x = (int)lua_tointeger(obj_script, -4);
			frame_rect.y = (int)lua_tointeger(obj_script, -3);
			passable = lua_toboolean(obj_script, -2);
			opaque = lua_toboolean(obj_script, -1);
			lua_pop(obj_script, 4);
		}
		else std::cout << "lua error: " << lua_tostring(obj_script, -1) << std::endl;
	}
	else std::cout << "attempting to interact with uninitialized object!" << std::endl;
}
void Object::update()
{
	if (!updated) return;

	if (obj_script != nullptr)
	{
		lua_getglobal(obj_script, "update");
		lua_pushnumber(obj_script, grid_x);
		lua_pushnumber(obj_script, grid_y);
		lua_pushnumber(obj_script, frame_rect.x);

		if (lua_pcall(obj_script, 3, 1, 0) == LUA_OK)
		{
			frame_rect.x = (int)lua_tointeger(obj_script, -1);
			lua_pop(obj_script, 1);
		}
		else std::cout << "lua error: " << lua_tostring(obj_script, -1) << std::endl;
	}
	else std::cout << "attempting to update an uninitialized object!" << std::endl;
}
void Object::animate()
{
	if (animated)
	{
		if (frame_rect.y == 0)
			frame_rect.y = 16;
		else frame_rect.y = 0;
	}
}
void Object::save_data(const std::string &data)
{
	if (obj_script != nullptr)
	{
		lua_getglobal(obj_script, "save_data");
		lua_pushnumber(obj_script, grid_x);
		lua_pushnumber(obj_script, grid_y);
		lua_pushstring(obj_script, data.c_str());

		if (lua_pcall(obj_script, 3, 0, 0) != LUA_OK)
			std::cout << "lua error: " << lua_tostring(obj_script, -1) << std::endl;
	}
	else std::cout << "attempting to save data to an uninitialized object!" << std::endl;
}
void Object::toggle_inventory(int8_t toggle)
{
	if (toggle > 0)
		ui.get_inventory(INV_TEMP)->set_source(this);
	else ui.get_inventory(INV_TEMP)->set_source(nullptr);

	ui.toggle_inventory(toggle, 0);
}
void Object::render_inventory_item(uint8_t slot, uint16_t xpos, uint16_t ypos, bool info) const
{
	if (inventory.capacity() >= slot && inventory[slot] != nullptr)
	{
		inventory[slot]->render(xpos, ypos, info);

		if (info && ui.get_temp_item() != nullptr)
			ui.get_temp_item()->render_info(xpos - 160, ypos);
	}
}
Item* Object::get_inventory_item(uint8_t slot) const
{
	Item *item = nullptr;
	if (inventory.capacity() >= slot)
		item = inventory[slot];

	return item;
}
void Object::set_inventory_item(uint8_t slot, Item *item)
{
	if (inventory.capacity() >= slot)
		inventory[slot] = item;
}
