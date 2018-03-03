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
#include "ability.hpp"
#include "actor.hpp"
#include "button.hpp"
#include "dungeon.hpp"

#include "player.hpp"
#include "camera.hpp"
#include "functions_ability.hpp"
#include "functions_general.hpp"
#include "lua_manager.hpp"
#include "level.hpp"
#include "texture_manager.hpp"
#include "texture.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

#include <algorithm> // for std::replace

SDL_Color string_to_color(const std::string &color)
{
	switch (djb_hash(color.c_str()))
	{
		case djb_hash("black"):      return COLOR_BLACK;      case djb_hash("plum"):       return COLOR_PLUM;
		case djb_hash("midnight"):   return COLOR_MIDNIGHT;   case djb_hash("iron"):       return COLOR_IRON;
		case djb_hash("earth"):      return COLOR_EARTH;      case djb_hash("moss"):       return COLOR_MOSS;
		case djb_hash("berry"):      return COLOR_BERRY;      case djb_hash("olive"):      return COLOR_OLIVE;
		case djb_hash("cornflower"): return COLOR_CORNFLOWER; case djb_hash("ocher"):      return COLOR_OCHER;
		case djb_hash("slate"):      return COLOR_SLATE;      case djb_hash("leaf"):       return COLOR_LEAF;
		case djb_hash("peach"):      return COLOR_PEACH;      case djb_hash("sky"):        return COLOR_SKY;
		case djb_hash("maize"):      return COLOR_MAIZE;      case djb_hash("peppermint"): return COLOR_PEPPERMINT;
		default:                     return COLOR_BLACK;
	}
	return COLOR_PEPPERMINT; // Should not ever get here
}
AbilityTarget string_to_target(const std::string &type)
{
	switch (djb_hash(type.c_str()))
	{
		case djb_hash("direction"): return AT_DIRECTION; case djb_hash("enemy"): return AT_ENEMY;
		case djb_hash("friendly"):  return AT_FRIENDLY;  case djb_hash("area"):  return AT_AREA;
		default:                    return AT_NONE;
	}
	return AT_NONE; // Should not ever get here
}
DirectionFilter string_to_filter(const std::string &type)
{
	switch (djb_hash(type.c_str()))
	{
		case djb_hash("floor"): return DF_FLOOR; case djb_hash("wall"):   return DF_WALL;
		case djb_hash("actor"): return DF_ACTOR; case djb_hash("object"): return DF_OBJECT;
		default:                return DF_NONE;
	}
	return DF_NONE; // Should not ever get here
}
PassiveTrigger string_to_trigger(const std::string &type)
{
	switch (djb_hash(type.c_str()))
	{
		case djb_hash("movement"): return PT_MOVEMENT;
		default:                   return PT_NONE;
	}
	return PT_NONE; // Should not ever get here
}
uint8_t button_to_direction(uint8_t direction)
{
	switch (direction)
	{
		case 0: return 1; case 1: return 2; case 2: return 4; case 3: return 8;
		case 4: return 5; case 5: return 9; case 6: return 6; case 7: return 10;
		default: return UINT8_MAX;
	}
}
Ability::Ability() :
	activated(false), ability_name("???"), ability_desc("???"), hotkey_name("?"),
	ability_target(AT_NONE), direction_filter(DF_NONE), passive_trigger(PT_NONE),
	ability_texture(nullptr), ability_script(nullptr), info_texture(nullptr)
{
	ability_rect = { 0, 0, 48, 48 };
	cooldown = std::make_pair(0, 0);

	for (uint8_t i = 0; i < 8; i++)
		buttons[i] = nullptr;
}
Ability::~Ability()
{
	free();
}
void Ability::free()
{
	if (ability_texture != nullptr)
	{
		SDL_DestroyTexture(ability_texture);
		ability_texture = nullptr;
	}
	for (uint8_t i = 0; i < 8; i++)
	{
		if (buttons[i] != nullptr)
			delete buttons[i];
		buttons[i] = nullptr;
	}
	if (ability_script != nullptr)
	{
		engine.get_lua_manager()->free_script(script_name);
		ability_script = nullptr;
	}
	if (info_texture != nullptr)
	{
		SDL_DestroyTexture(info_texture);
		info_texture = nullptr;
	}
}
bool Ability::init(Dungeon *scene, const std::string &script, SDL_Keycode code)
{
	ability_script = engine.get_lua_manager()->load_script(script);
	if (ability_script != nullptr)
	{
		game_scene = scene;
		script_name = script;

		if (engine.get_lua_manager()->get_reference_count(script) < 2)
		{
			register_ability_functions(ability_script);
			register_general_functions(ability_script);
		}
		lua_getglobal(ability_script, "register");
		if (lua_pcall(ability_script, 0, 1, 0) == LUA_OK)
		{
			std::string texture_name;
			SDL_Color ability_color;

			for (lua_pushnil(ability_script); lua_next(ability_script, -2); lua_pop(ability_script, 1))
			{
				const std::string key = lua_tostring(ability_script, -2);
				switch (djb_hash(key.c_str()))
				{
					case djb_hash("name"):
						ability_name = lua_tostring(ability_script, -1);
						//std::replace(ability_name.begin(), ability_name.end(), '_', ' ');
						break;
					case djb_hash("target"):
						set_target_type(string_to_target(lua_tostring(ability_script, -1)));
						break;
					case djb_hash("filter"):
						direction_filter = string_to_filter(lua_tostring(ability_script, -1));
						break;
					case djb_hash("texture"):
						texture_name = lua_tostring(ability_script, -1);
						break;
					case djb_hash("color"):
						ability_color = string_to_color(lua_tostring(ability_script, -1));
						break;
					case djb_hash("description"):
						ability_desc = lua_tostring(ability_script, -1);
						break;
					case djb_hash("trigger"):
						passive_trigger = string_to_trigger(lua_tostring(ability_script, -1));
						break;
					case djb_hash("cooldown"):
						cooldown = std::make_pair(0, lua_tointeger(ability_script, -1));
						break;
					default: std::cout << "undefined ability key: " << key << std::endl;
				}
			}
			if (init_texture(texture_name, ability_color, code))
				return true;
		}
		else std::cout << "lua error: " << lua_tostring(ability_script, -1) << std::endl;
	}
	return false;
}
bool Ability::init_texture(const std::string &icon, SDL_Color color, SDL_Keycode code)
{
	ability_texture = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 96, 48
	);
	if (ability_texture == NULL)
	{
		ability_texture = nullptr;
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	if (ui.get_background() != nullptr)
	{
		SDL_SetRenderTarget(engine.get_renderer(), ability_texture);

		SDL_Rect center = { 20, 20, 8, 8 };
		SDL_Rect corners[4] =
		{
			{ 0, 0, 16, 8 }, // Top left
			{ 40, 0, 8, 16 }, // Top right
			{ 0, 32, 8, 16 }, // Bottom left
			{ 32, 40, 16, 8 } // Bottom right
		};
		Texture *temp_texture = engine.get_texture_manager()->load_texture(icon, true);
		if (temp_texture != nullptr)
			temp_texture->set_color(color);

		//const uint8_t prev_scale = ui.get_bitmap_font()->get_scale();
		//ui.get_bitmap_font()->set_scale(2);

		hotkey = code;
		hotkey_name = "";

		if (hotkey != NULL)
			hotkey_name = SDL_GetKeyName(hotkey);

		for (uint8_t i = 0; i < 2; i++)
		{
			if (i == 1)
			{
				for (uint8_t j = 0; j < 4; j++)
					corners[j].x += 64;
				center.x += 64;
			}
			ui.get_background()->render(i * 48 + 16, 16, &center);

			ui.get_background()->render(i * 48, 0, &corners[0]);
			ui.get_background()->render(i * 48 + 32, 0, &corners[1]);
			ui.get_background()->render(i * 48, 16, &corners[2]);
			ui.get_background()->render(i * 48 + 16, 32, &corners[3]);

			if (temp_texture != nullptr)
				temp_texture->render(i * 48 + 8, 8);

			//ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
			//ui.get_bitmap_font()->render_text(i * 48 + 30, 28, hotkey_name);
		}
		if (temp_texture != nullptr)
			engine.get_texture_manager()->free_texture(temp_texture->get_name());
		//ui.get_bitmap_font()->set_scale(prev_scale);

		//SDL_SetRenderTarget(engine.get_renderer(), NULL);

		info_texture = SDL_CreateTexture(engine.get_renderer(),
			SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 160, 128
		);
		if (info_texture == NULL)
		{
			info_texture = nullptr;
			std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
			SDL_SetRenderTarget(engine.get_renderer(), NULL);
			return false;
		}
		SDL_SetRenderTarget(engine.get_renderer(), info_texture);

		std::string display_name = ability_name;
		std::replace(display_name.begin(), display_name.end(), '_', ' ');

		ui.draw_box(0, 0, 5, 4, true);
		ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
		ui.get_bitmap_font()->render_text(16, 16, display_name);
		ui.get_bitmap_font()->render_text(16, 38, ability_desc, 15);
		ui.get_bitmap_font()->render_text(16, 104, "cooldown:");
		if (cooldown.second == 0)
			ui.get_bitmap_font()->render_text(112, 104, "none");

		ui.get_bitmap_font()->set_color(COLOR_OLIVE);
		const std::string sep = std::string(16, '-');
		ui.get_bitmap_font()->render_text(16, 27, sep);
		ui.get_bitmap_font()->render_text(16, 93, sep);

		SDL_SetRenderTarget(engine.get_renderer(), NULL);
		return true;
	}
	return false;
}
void Ability::render() const
{
	if (ability_texture != nullptr)
	{
		const SDL_Rect quad = { x, y, 48, 48 };
		SDL_RenderCopyEx(engine.get_renderer(), ability_texture, &ability_rect, &quad, 0.0, nullptr, SDL_FLIP_NONE);

		const uint8_t prev_scale = ui.get_bitmap_font()->get_scale();
		ui.get_bitmap_font()->set_scale(2);
		ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
		if (cooldown.first > 0)
		{
			const uint8_t c = (cooldown.first > 4) ? 155 : 160 - cooldown.first;
			ui.get_bitmap_font()->render_char(x + 30, y + 28, c);
		}
		else ui.get_bitmap_font()->render_char(x + 30, y + 28, hotkey_name[0]);
		ui.get_bitmap_font()->set_scale(prev_scale);

		if (activated)
		{
			const int16_t render_x = player->get_x() - camera.get_cam_x();
			const int16_t render_y = player->get_y() - camera.get_cam_y();

			switch (ability_target)
			{
				case AT_DIRECTION:
					for (uint8_t i = 0; i < 8; i++)
					{
						if (buttons[i] != nullptr && buttons[i]->get_enabled())
							buttons[i]->render();
					}
					break;
				default: break;
			}
		}
		if (ability_rect.x != 0 && info_texture != nullptr)
		{
			const SDL_Rect clip = { 0, 0, 160, 128 };
			const SDL_Rect quad = { x + 24, y + 24, 160, 128 };

			SDL_RenderCopyEx(engine.get_renderer(), info_texture, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);

			// Cooldown is constantly changing, so it's rendered separately from the base texture
			if (cooldown.second != 0)
			{
				const std::string cd = std::to_string(cooldown.first) + "/" + std::to_string(cooldown.second);
				ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
				ui.get_bitmap_font()->render_text(x + 168 - (cd.length() * 8), y + 128, cd);
			}
		}
	}
}
void Ability::activate()
{
	if (cooldown.first > 0) return;

	if (ability_target == AT_NONE || ability_target == AT_PASSIVE)
	{
		apply(player);
		return;
	}
	activated = true;
	ability_rect.x = 48;

	if (ability_target == AT_DIRECTION)
		update_direction_buttons();

	player->set_ability(this);
}
void Ability::apply(Actor *source, uint8_t value)
{
	if (ability_script != nullptr)
	{
		std::cout << "applying ability with value '" << (int)value << "'!" << std::endl;

		if (ability_target == AT_PASSIVE && passive_trigger == PT_MOVEMENT)
			value = source->get_direction();

		lua_getglobal(ability_script, "apply");
		lua_pushnumber(ability_script, value);

		// All the logic happens in the script and the functions it calls
		if (lua_pcall(ability_script, 1, 0, 0) != LUA_OK)
			std::cout << "lua error: " << lua_tostring(ability_script, -1) << std::endl;
	}
	else std::cout << "attempting to apply uninitialized ability!" << std::endl;

	activated = false;
	ability_rect.x = 0;

	// The player's turn should always end after an action,
	// so we add 1 to the cooldown to actually start it after the current turn
	// (since all cooldowns get reduced by 1 at the end of the turn)
	if (cooldown.second > 0)
		cooldown.first = cooldown.second + 1;

	if (source == player)
		player->set_ability(nullptr);
}
bool Ability::validate_value(uint8_t value)
{
	if (ability_target == AT_DIRECTION)
	{
		if (value < 8 && buttons[value] != nullptr)
			return buttons[value]->get_enabled();
	}
	return false;
}
void Ability::cancel()
{
	std::cout << "ability cancelled!" << std::endl;

	activated = false;
	ability_rect.x = 0;

	player->set_ability(nullptr);
}
void Ability::reduce_cooldown(uint8_t amount)
{
	if (cooldown.first >= amount)
		cooldown.first -= amount;
	else cooldown.first = 0;
}
void Ability::set_target_type(uint8_t type)
{
	ability_target = type;
	switch (type)
	{
		case AT_DIRECTION: init_direction_buttons(); break;
		default: std::cout << "warning: ability '" << ability_name << "' has unknown target type!" << std::endl; break;
	}
}
bool Ability::get_overlap(int16_t xpos, int16_t ypos)
{
	if (activated)
	{
		bool overlap = false;
		if (ability_target == AT_DIRECTION) for (uint8_t i = 0; i < 8; i++)
		{
			if (buttons[i] != nullptr && buttons[i]->get_enabled())
				overlap = buttons[i]->get_overlap(xpos, ypos);
		}
		if (overlap) return true;
	}
	if (xpos > x && xpos < (x + 48) && ypos > y && ypos < (y + 48))
		ability_rect.x = 48;
	else if (!activated)
		ability_rect.x = 0;

	return ability_rect.x != 0;
}
bool Ability::get_click()
{
	if (!activated && ability_rect.x != 0)
		activate();

	else if (activated && ability_target == AT_DIRECTION)
	{
		bool button = false;
		uint8_t i = 0; for (i; i < 8; i++)
		{
			if (buttons[i] != nullptr && buttons[i]->get_click())
			{
				button = true;
				break;
			}
		}
		if (button)
			apply(player, button_to_direction(i));
		else cancel();
		return true;
	}
	else activated = false;
	return activated;
}
void Ability::init_direction_buttons()
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (buttons[i] != nullptr)
			delete buttons[i];

		buttons[i] = new Button;
		buttons[i]->init(BTN_DIRECTION_KEY, i);
		buttons[i]->set_enabled(false);
		buttons[i]->set_consider_camera(true);
	}
}
void Ability::update_direction_buttons()
{
	if (game_scene->get_level() == nullptr)
	{
		std::cout << "could not update ability direction buttons!" << std::endl;
		return;
	}
	const int8_t offset_x[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	const int8_t offset_y[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };

	for (uint8_t i = 0; i < 8; i++)
	{
		if (buttons[i] != nullptr)
		{
			bool enable_button = false;
			if (direction_filter == DF_NONE) // Button gets always enabled
			{
				enable_button = true;
			}
			else if (direction_filter == DF_FLOOR) // Button gets enabled if there's no wall under it
			{
				enable_button = !(game_scene->get_level()->get_wall(
					player->get_grid_x() + offset_x[i],
					player->get_grid_y() + offset_y[i]
				));
			}
			else if (direction_filter == DF_WALL) // Button gets enabled if there IS a wall under it
			{
				enable_button = game_scene->get_level()->get_wall(
					player->get_grid_x() + offset_x[i],
					player->get_grid_y() + offset_y[i]
				);
			}
			else if (direction_filter == DF_ACTOR) // Button gets enabled if there's an actor under it
			{
				enable_button = (game_scene->get_level()->get_actor(
					player->get_grid_x() + offset_x[i],
					player->get_grid_y() + offset_y[i]
				) != nullptr);
			}
			else if (direction_filter == DF_OBJECT) // Button gets enabled if there's an object under it
			{
				enable_button = (game_scene->get_level()->get_object(
					player->get_grid_x() + offset_x[i],
					player->get_grid_y() + offset_y[i]
				) != nullptr);
			}
			buttons[i]->set_enabled(enable_button);
			if (enable_button)
			{
				buttons[i]->set_position(
					player->get_x() + (offset_x[i] * 32),
					player->get_y() + (offset_y[i] * 32)
				);
			}
		}
	}
}
