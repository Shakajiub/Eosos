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

#ifndef UI_HPP
#define UI_HPP

class Actor;
class Item;
class Texture;
class BitmapFont;
class ExperienceBar;
class HealthIndicator;
class Inventory;
class MessageLog;

enum HealthBar
{
	HB_PLAYER,
	HB_OTHER,
	HB_BOSS
};
enum InventoryType
{
	INV_TEMP,
	INV_EQUIP,
	INV_BACKPACK
};
class UI
{
public:
	UI();
	~UI();

	void free();

	bool init_bitmap_font();
	void init_experience_bar();
	void init_healthbar(HealthBar hb);
	void init_inventory(InventoryType it);
	void init_message_log();

	void update();
	void render(int16_t mouse_x, int16_t mouse_y) const;

	void toggle_inventory(int8_t toggle = -1, int8_t id = -1);
	void draw_box(uint16_t xpos, uint16_t ypos, uint8_t width, uint8_t height, bool highlight = false) const;

	bool input_controller(uint8_t index, uint8_t value);
	void give_controller_input(uint8_t id);

	bool get_overlap(int16_t xpos, int16_t ypos) const;
	bool get_click(int16_t xpos, int16_t ypos) const;

	Item* get_temp_item() const { return temp_item; }
	Texture* get_background() const { return ui_background; }
	BitmapFont* get_bitmap_font() const { return main_font; }
	ExperienceBar* get_experience_bar() const { return experience_bar; }
	HealthIndicator* get_healthbar(HealthBar hb) const { return healthbars[hb]; }
	Inventory* get_inventory(InventoryType it) const { return inventories[it]; }
	MessageLog* get_message_log() const { return message_log; }

	void set_temp_item(Item *item) { temp_item = item; }
	void set_healthbar_target(HealthBar hb, Actor *target);

private:
	Item *temp_item;
	Texture *ui_background;
	BitmapFont *main_font;
	ExperienceBar *experience_bar;
	HealthIndicator* healthbars[3];
	Inventory* inventories[3];
	MessageLog *message_log;
};
extern UI ui;

#endif // UI_HPP
