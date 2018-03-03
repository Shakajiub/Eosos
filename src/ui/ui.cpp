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
#include "ui.hpp"
#include "actor.hpp"
#include "item.hpp"
#include "texture.hpp"
#include "bitmap_font.hpp"
#include "experience_bar.hpp"
#include "health_indicator.hpp"
#include "inventory.hpp"
#include "message_log.hpp"

#include "ability_manager.hpp"
#include "player.hpp"
#include "camera.hpp"
#include "options.hpp"
#include "texture_manager.hpp"

UI ui;

UI::UI() :
	temp_item(nullptr), ui_background(nullptr), main_font(nullptr), experience_bar(nullptr), message_log(nullptr)
{
	for (uint8_t i = 0; i < 3; i++)
	{
		healthbars[i] = nullptr;
		inventories[i] = nullptr;
	}
}
UI::~UI()
{
	free();
}
void UI::free()
{
	if (temp_item != nullptr)
	{
		delete temp_item;
		temp_item = nullptr;
	}
	if (ui_background != nullptr)
	{
		engine.get_texture_manager()->free_texture(ui_background->get_name());
		ui_background = nullptr;
	}
	if (main_font != nullptr)
	{
		delete main_font;
		main_font = nullptr;
	}
	if (experience_bar != nullptr)
	{
		delete experience_bar;
		experience_bar = nullptr;
	}
	if (message_log != nullptr)
	{
		delete message_log;
		message_log = nullptr;
	}
	for (uint8_t i = 0; i < 3; i++)
	{
		if (healthbars[i] != nullptr)
			delete healthbars[i];
		healthbars[i] = nullptr;

		if (inventories[i] != nullptr)
			delete inventories[i];
		inventories[i] = nullptr;
	}
}
bool UI::init_bitmap_font()
{
	main_font = new BitmapFont;
	if (!main_font->build("core/texture/ui/" + options.get_s("ui-font") + ".png"))
		return false;
	return true;
}
void UI::init_experience_bar()
{
	if (ui_background == nullptr)
	{
		ui_background = engine.get_texture_manager()->load_texture(
			"core/texture/ui/" + options.get_s("ui-image") + ".png"
		);
	}
	experience_bar = new ExperienceBar;
	experience_bar->set_position(0, camera.get_cam_h() - (options.get_i("ui-log_height") * 32) - 32);
	experience_bar->init();
}
void UI::init_healthbar(HealthBar hb)
{
	switch (hb)
	{
		case HB_PLAYER:
			healthbars[hb] = new HealthIndicator;
			healthbars[hb]->init(player, "core/texture/ui/heart_normal.png");
			healthbars[hb]->set_position(16, 64);
			break;
		case HB_OTHER:
			healthbars[hb] = new HealthIndicator;
			healthbars[hb]->init(nullptr, "core/texture/ui/heart_black.png");
			healthbars[hb]->set_position(16, 112);
		default: break;
	}
}
void UI::init_inventory(InventoryType it)
{
	uint8_t inventory_width = 3;
	uint8_t inventory_height = 4;
	std::vector<uint8_t> equipment_limits;

	if (it == INV_EQUIP && player != nullptr)
	{
		lua_State *script = player->get_class_script();
		if (script != nullptr)
		{
			// Get the inventory size
			lua_getglobal(script, "get_inventory_size");
			if (lua_pcall(script, 0, 2, 0) == LUA_OK)
			{
				inventory_width = (uint8_t)lua_tointeger(script, -2);
				inventory_height = (uint8_t)lua_tointeger(script, -1);
				lua_pop(script, 2);
			}
			else std::cout << "lua error: " << lua_tostring(script, -1) << std::endl;

			// Get equipment slot limits
			lua_getglobal(script, "get_class_equipment");
			if (lua_pcall(script, 0, 1, 0) == LUA_OK)
			{
				for (lua_pushnil(script); lua_next(script, -2); lua_pop(script, 1))
					equipment_limits.push_back(lua_tointeger(script, -1));
			}
			else std::cout << "lua error: " << lua_tostring(script, -1) << std::endl;
		}
		else std::cout << "player doesn't have a class script!" << std::endl;
	}
	switch (it)
	{
		case INV_TEMP:
			if (inventories[it] != nullptr)
				delete inventories[it];
			inventories[it] = new Inventory;
			inventories[it]->init(INV_TEMP, "Container", 5, 3);
			inventories[it]->set_position(camera.get_cam_w() - 512, camera.get_cam_h() - 224);
			break;
		case INV_EQUIP:
			inventories[it] = new Inventory;
			inventories[it]->init(INV_EQUIP, "Equip", inventory_width, inventory_height);
			inventories[it]->set_position(camera.get_cam_w() - 160, camera.get_cam_h() - 448);
			for (uint8_t i = 0; i < equipment_limits.size(); i++)
				inventories[it]->limit_slot(i, equipment_limits[i]);
			break;
		case INV_BACKPACK:
			inventories[it] = new Inventory;
			inventories[it]->init(INV_BACKPACK, "Backpack", 7, 3);
			inventories[it]->set_position(camera.get_cam_w() - 288, camera.get_cam_h() - 224);
			break;
		default: break;
	}
}
void UI::init_message_log()
{
	if (ui_background == nullptr)
	{
		ui_background = engine.get_texture_manager()->load_texture(
			"core/texture/ui/" + options.get_s("ui-image") + ".png"
		);
	}
	message_log = new MessageLog;
	message_log->set_position(0, camera.get_cam_h() - (options.get_i("ui-log_height") * 32));
	message_log->set_size((uint8_t)options.get_i("ui-log_width"), (uint8_t)options.get_i("ui-log_height"));
	message_log->init();
	message_log->add_message("Welcome to HELL.", COLOR_BERRY);
}
void UI::update()
{
	for (uint8_t i = 0; i < 3; i++)
	{
		if (healthbars[i] != nullptr)
			healthbars[i]->update();
	}
}
void UI::render(int16_t mouse_x, int16_t mouse_y) const
{
	if (message_log != nullptr)
		message_log->render();
	if (experience_bar != nullptr)
		experience_bar->render();

	for (uint8_t i = 0; i < 3; i++)
	{
		if (healthbars[i] != nullptr)
			healthbars[i]->render();
		if (inventories[i] != nullptr)
			inventories[i]->render();
	}
	player->get_ability_manager()->render_abilities();

	if (temp_item != nullptr)
		temp_item->render(mouse_x - 16, mouse_y - 16);
}
void UI::toggle_inventory(int8_t toggle, int8_t id)
{
	if (id != -1) // Toggle a specific inventory
	{
		if (inventories[id] != nullptr)
		{
			bool flag = inventories[id]->get_render_flag();
			flag = toggle < 0 ? !flag : toggle; // If we are not given a state, flip the flag
			inventories[id]->set_render_flag(flag);
		}
		return;
	}
	for (uint8_t i = 1; i < 3; i++) // Toggle equipment & backpack
	{
		if (inventories[i] != nullptr)
		{
			bool flag = inventories[i]->get_render_flag();
			flag = toggle < 0 ? !flag : toggle; // If we are not given a state, flip the flag
			inventories[i]->set_render_flag(flag);
		}
	}
}
void UI::draw_box(uint16_t xpos, uint16_t ypos, uint8_t width, uint8_t height, bool highlight) const
{
	if (ui_background == nullptr)
	{
		std::cout << "ui error: no proper background texture!" << std::endl;
		return;
	}
	if (width == 1) // If the width is 1, just draw a vertical "bar"
	{
		for (uint8_t i = 0; i < height + 1; i++)
		{
			SDL_Rect temp_rect = { 48, 0, 16, 16 };
			if (i == width)
				temp_rect.y = 32;
			else if (i != 0)
				temp_rect.y = 16;

			if (highlight || options.get_b("ui-highlight"))
				temp_rect.x += 64;
			ui_background->render(0, i * 32, &temp_rect);
		}
		return;
	}
	if (height == 1) // Same for the height, but a horizontal "bar"
	{
		for (uint8_t i = 0; i < width + 1; i++)
		{
			SDL_Rect temp_rect = { 0, 48, 16, 16 };
			if (i == width)
				temp_rect.x = 32;
			else if (i != 0)
				temp_rect.x = 16;

			if (highlight || options.get_b("ui-highlight"))
				temp_rect.x += 64;
			ui_background->render(i * 32, 0, &temp_rect);
		}
		return;
	}
	for (uint8_t x = 0; x < width; x++) // Otherwise, just draw the box
	{
		for (uint8_t y = 0; y < height; y++)
		{
			SDL_Rect temp_rect = { 16, 16, 16, 16 };
			if (x == 0)
			{
				if (y == 0) temp_rect = { 0, 0, 16, 16};
				else if (y == height - 1) temp_rect = { 0, 32, 16, 16 };
				else temp_rect = { 0, 16, 16, 16 };
			}
			else if (x == width - 1)
			{
				if (y == 0) temp_rect = { 32, 0, 16, 16};
				else if (y == height - 1) temp_rect = { 32, 32, 16, 16 };
				else temp_rect = { 32, 16, 16, 16 };
			}
			else if (y == 0)
				temp_rect = { 16, 0, 16, 16 };
			else if (y == height - 1)
				temp_rect = { 16, 32, 16, 16 };

			if (highlight || options.get_b("ui-highlight"))
				temp_rect.x += 64;
			ui_background->render(xpos + (x * 32), ypos + (y * 32), &temp_rect);
		}
	}
}
bool UI::input_controller(uint8_t index, uint8_t value)
{
	bool got_input = false;
	for (uint8_t i = 0; i < 3; i++)
	{
		if (inventories[i] != nullptr)
		{
			got_input = inventories[i]->input_controller(index, value);
			if (got_input)
				break;
		}
	}
	return got_input;
}
void UI::give_controller_input(uint8_t id)
{
	if (id < 3 && inventories[id] != nullptr)
		inventories[id]->give_controller_input();
}
bool UI::get_overlap(int16_t xpos, int16_t ypos) const
{
	for (uint8_t i = 0; i < 3; i++)
	{
		if (inventories[i] != nullptr && inventories[i]->get_overlap(xpos, ypos))
			return true;
	}
	if (player->get_ability_manager()->overlap_abilities(xpos, ypos))
		return true;
	if (message_log != nullptr && message_log->get_overlap(xpos, ypos))
		return true;
	if (experience_bar != nullptr && experience_bar->get_overlap(xpos, ypos))
		return true;
	return false;
}
bool UI::get_click(int16_t xpos, int16_t ypos) const
{
	for (uint8_t i = 0; i < 3; i++)
	{
		if (inventories[i] != nullptr && inventories[i]->get_click(xpos, ypos))
			return true;
	}
	if (player->get_ability_manager()->click_abilities())
		return true;
	return false;
}
void UI::set_healthbar_target(HealthBar hb, Actor *target)
{
	if (healthbars[hb] != nullptr)
		healthbars[hb]->set_target(target);
}
