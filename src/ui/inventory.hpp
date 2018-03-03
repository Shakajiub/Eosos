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

#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include <vector>

class Item;
class Object;
class Texture;
class Button;

class Inventory
{
public:
	Inventory();
	~Inventory();

	void free();
	void init(uint8_t id, const std::string &title, uint8_t inv_width, uint8_t inv_height);

	void render() const;
	void clear();

	void push_item(Item *item);
	void limit_slot(uint8_t slot, uint8_t type, const std::string &custom_texture = "");
	void modify_durability(uint8_t item_type, int8_t value);

	bool input_controller(uint8_t index, uint8_t value);
	void give_controller_input();

	bool get_render_flag() const { return render_flag; }
	bool get_overlap_flag() const { return overlap_flag; }
	bool get_overlap(int16_t xpos, int16_t ypos);
	bool get_click(int16_t xpos, int16_t ypos);

	void set_render_flag(bool flag);
	void set_position(int16_t xpos, int16_t ypos);
	void set_source(Object *obj) { inv_source = obj; }

private:
	bool render_flag;
	bool overlap_flag;

	uint8_t ID;
	int16_t x, y;
	uint8_t width, height;
	int8_t slot_x, slot_y;
	uint16_t start_x, start_y;
	uint8_t button_offset;

	std::vector<Item*> inventory;
	std::vector<std::pair<uint8_t, uint8_t> > limits;

	SDL_Texture *inv_texture;
	Texture *slot_highlight;
	Button *close_button;
	Object *inv_source;
};

#endif // INVENTORY_HPP
