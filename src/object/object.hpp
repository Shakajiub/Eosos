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

#ifndef OBJECT_HPP
#define OBJECT_HPP

class Item;
class Texture;

class Object
{
public:
	Object();
	~Object();

	void free();
	void render() const;

	bool construct(uint8_t xpos, uint8_t ypos, const std::string &script, const std::string &type);
	void interact();
	void update();
	void animate();
	void save_data(const std::string &data);

	void toggle_inventory(int8_t toggle = -1);
	void render_inventory_item(uint8_t slot, uint16_t xpos, uint16_t ypos, bool info = false) const;

	bool get_passable() const { return passable; }
	bool get_opaque() const { return opaque; }
	bool get_delete() const { return delete_flag; }
	uint8_t get_grid_x() const { return grid_x; }
	uint8_t get_grid_y() const { return grid_y; }
	Item* get_inventory_item(uint8_t slot) const;

	void set_delete(bool flag) { delete_flag = flag; }
	void set_inventory_item(uint8_t slot, Item *item);

protected:
	bool passable, opaque;
	bool animated, updated;
	bool delete_flag;
	uint8_t grid_x, grid_y;

	std::string obj_script_name;
	SDL_Rect frame_rect;

	lua_State *obj_script;
	Texture *obj_texture;

	std::vector<Item*> inventory;
};

#endif // OBJECT_HPP
