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
#include "item.hpp"
#include "texture.hpp"

#include "lua_manager.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

#include <algorithm> // for std::replace

EquipType string_to_equip(const std::string &type)
{
	switch (djb_hash(type.c_str()))
	{
		case djb_hash("ammo"):   return ET_AMMO;   case djb_hash("amulet"): return ET_AMULET;
		case djb_hash("armor"):  return ET_ARMOR;  case djb_hash("boots"):  return ET_BOOTS;
		case djb_hash("cloak"):  return ET_CLOAK;  case djb_hash("gloves"): return ET_GLOVES;
		case djb_hash("hat"):    return ET_HAT;    case djb_hash("light"):  return ET_LIGHT;
		case djb_hash("shield"): return ET_SHIELD; case djb_hash("tool"):   return ET_TOOL;
		case djb_hash("ring"):   return ET_RING;   case djb_hash("weapon"): return ET_WEAPON;
		default:                 return ET_NONE;
	}
	return ET_NONE; // Should not ever get here
}
MiscType string_to_misc(const std::string &type)
{
	switch (djb_hash(type.c_str()))
	{
		case djb_hash("book"):   return MT_BOOK;   case djb_hash("food"):     return MT_FOOD;
		case djb_hash("gem"):    return MT_GEM;    case djb_hash("material"): return MT_MATERIAL;
		case djb_hash("money"):  return MT_MONEY;  case djb_hash("potion"):   return MT_POTION;
		case djb_hash("scroll"): return MT_SCROLL; default:                   return MT_NONE;
	}
	return MT_NONE; // Should not ever get here
}
WeaponType string_to_weapon(const std::string &type)
{
	switch (djb_hash(type.c_str()))
	{
		case djb_hash("axe"):    return WT_AXE;    case djb_hash("dagger"):  return WT_DAGGER;
		case djb_hash("hammer"): return WT_HAMMER; case djb_hash("polearm"): return WT_POLEARM;
		case djb_hash("ranged"): return WT_RANGED; case djb_hash("shield"):  return WT_SHIELD;
		case djb_hash("sword"):  return WT_SWORD;  case djb_hash("wand"):    return WT_WAND;
		default:                 return WT_NONE;
	}
	return WT_NONE; // Should not ever get here
}
ItemSize string_to_size(const std::string &type)
{
	switch (djb_hash(type.c_str()))
	{
		case djb_hash("small"): return IS_SMALL; case djb_hash("medium"): return IS_MEDIUM;
		case djb_hash("large"): return IS_LARGE; default:                 return IS_NONE;
	}
	return IS_NONE; // Should not ever get here
}
DamageType string_to_damage(const std::string &type)
{
	switch (djb_hash(type.c_str()))
	{
		case djb_hash("blunt"):    return DT_BLUNT; case djb_hash("piercing"): return DT_PIERCE;
		case djb_hash("slashing"): return DT_SLASH; default:                   return DT_NONE;
	}
	return DT_NONE; // Should not ever get here
}
Item::Item() :
	item_in_use(false), item_name("???"), item_script(""), item_texture(nullptr), info_texture(nullptr),
	equip_type(ET_NONE), misc_type(MT_NONE), weapon_type(WT_NONE), item_size(IS_NONE)
{
	durability = std::make_pair(32, 32);
}
Item::~Item()
{
	free();
}
void Item::free()
{
	if (item_texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(item_texture->get_name());
		item_texture = nullptr;
	}
	if (info_texture != nullptr)
	{
		SDL_DestroyTexture(info_texture);
		info_texture = nullptr;
	}
	item_values.clear();
}
bool Item::init(const std::string &script_name, const std::string &item_type, bool free_script)
{
	free();

	lua_State *script = engine.get_lua_manager()->load_script(script_name);
	if (script != nullptr)
	{
		uint8_t temp_value, temp_armor;
		std::string temp_equip, temp_damage, temp_misc, temp_weapon, temp_size, temp_type;
		std::size_t dice_split, extra_split;

		lua_getglobal(script, "get_item_data");
		lua_pushstring(script, item_type.c_str());

		if (lua_pcall(script, 1, 1, 0) == LUA_OK)
		{
			for (lua_pushnil(script); lua_next(script, -2); lua_pop(script, 1))
			{
				const std::string key = lua_tostring(script, -2);
				switch (djb_hash(key.c_str()))
				{
					case djb_hash("name"):
						item_name = lua_tostring(script, -1);
						std::replace(item_name.begin(), item_name.end(), '_', ' ');
						break;
					case djb_hash("texture"):
						item_texture = engine.get_texture_manager()->load_texture(lua_tostring(script, -1));
						break;
					case djb_hash("script"):
						item_script = lua_tostring(script, -1);
						break;
					case djb_hash("equip"):
						temp_equip = lua_tostring(script, -1);
						equip_type = string_to_equip(temp_equip);
						break;
					case djb_hash("damage"):
						temp_damage = lua_tostring(script, -1);
						item_values.clear();

						dice_split = temp_damage.find('d');
						extra_split = temp_damage.find('+');

						if (dice_split != std::string::npos)
						{
							item_values.push_back(std::stoi(temp_damage.substr(0, dice_split)));
							if (extra_split != std::string::npos)
							{
								item_values.push_back(std::stoi(temp_damage.substr(dice_split + 1, extra_split)));
								item_values.push_back(std::stoi(temp_damage.substr(extra_split + 1)));
							}
							else
							{
								item_values.push_back(std::stoi(temp_damage.substr(dice_split + 1)));
								item_values.push_back(0);
							}
						}
						break;
					case djb_hash("misc"):
						temp_misc = lua_tostring(script, -1);
						misc_type = string_to_misc(temp_misc);
						break;
					case djb_hash("weapon"):
						temp_weapon = lua_tostring(script, -1);
						weapon_type = string_to_weapon(temp_weapon);
						break;
					case djb_hash("size"):
						temp_size = lua_tostring(script, -1);
						item_size = string_to_size(temp_size);
						break;
					case djb_hash("type"):
						temp_type = lua_tostring(script, -1);
						damage_type = string_to_damage(temp_type);
						break;
					case djb_hash("armor"):
						temp_armor = lua_tointeger(script, -1);
						item_values.clear();
						item_values.push_back(temp_armor);
						break;
					case djb_hash("strength"):
						temp_value = lua_tointeger(script, -1);
						item_values.clear();
						item_values.push_back(temp_value);
						break;
					case djb_hash("durability"):
						temp_value = lua_tointeger(script, -1);
						durability = std::make_pair(temp_value, temp_value);
						break;
					default: std::cout << "undefined item key: " << key << std::endl; break;
				}
			}
		}
		else std::cout << "lua error: " << lua_tostring(script, -1) << std::endl;

		std::cout << "found item values";
		for (uint8_t i = 0; i < item_values.size(); i++)
			std::cout << ", " << (int)item_values[i];
		std::cout << std::endl;

		if (free_script)
			engine.get_lua_manager()->free_script(script_name);

		if (item_texture != nullptr)
		{
			info_texture = SDL_CreateTexture(engine.get_renderer(),
				SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 160, 128
			);
			if (info_texture == NULL)
			{
				info_texture = nullptr;
				std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
			}
			else // Draw all the info for the item's info box (displayed when hovered)
			{
				SDL_SetRenderTarget(engine.get_renderer(), info_texture);

				ui.draw_box(0, 0, 5, 4, true);
				ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
				ui.get_bitmap_font()->render_text(16, 16, item_name);
				ui.get_bitmap_font()->render_text(16, 38, "type:   " + (equip_type == ET_WEAPON ? temp_weapon : temp_equip));

				if (item_size != IS_NONE) // Draw item size for those that have it
				{
					ui.get_bitmap_font()->render_text(16, 49, "size:   " + temp_size);
				}
				if (equip_type == ET_WEAPON) // Draw weapon damage
				{
					ui.get_bitmap_font()->render_text(16, 60, "damage: " + temp_damage);
					ui.get_bitmap_font()->render_text(80, 71, temp_type);
				}
				else if ( // Draw armor value
					equip_type == ET_ARMOR || equip_type == ET_BOOTS ||
					equip_type == ET_CLOAK || equip_type == ET_GLOVES ||
					equip_type == ET_HAT   || equip_type == ET_SHIELD
				){
					ui.get_bitmap_font()->render_text(16, 60, "armor:  " + std::to_string(temp_armor));
				}
				else if (equip_type == ET_LIGHT) // Draw light strength
				{
					if (item_values.size() == 1)
						ui.get_bitmap_font()->render_text(16, 82, "strength: " + std::to_string(item_values[0]));
				}
				ui.get_bitmap_font()->render_text(16, 104, "durable:");

				ui.get_bitmap_font()->set_color(COLOR_OLIVE);
				const std::string sep = std::string(16, '-');
				ui.get_bitmap_font()->render_text(16, 27, sep);
				ui.get_bitmap_font()->render_text(16, 93, sep);

				SDL_SetRenderTarget(engine.get_renderer(), NULL);
			}
			std::cout << "item loaded: " << item_type << std::endl;
			return true;
		}
		else std::cout << "could not load item '" << item_type << "'!" << std::endl;
	}
	return false;
}
void Item::render(int16_t xpos, int16_t ypos, bool info) const
{
	if (item_texture != nullptr)
		item_texture->render(xpos, ypos);

	if (info && info_texture != nullptr)
		render_info(xpos, ypos);
}
void Item::render_info(int16_t xpos, int16_t ypos) const
{
	if (info_texture != nullptr)
	{
		const SDL_Rect clip = { 0, 0, 160, 128 };
		const SDL_Rect quad = { xpos - 160, ypos - 128, 160, 128 };

		SDL_RenderCopyEx(engine.get_renderer(), info_texture, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);

		// Durability is constantly changing, so it's rendered separately from the base texture
		const std::string dur = std::to_string(durability.first) + "/" + std::to_string(durability.second);
		ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
		ui.get_bitmap_font()->render_text(xpos - 16 - (dur.length() * 8), ypos - 24, dur);
	}
}
uint8_t Item::get_value(uint8_t index) const
{
	if (item_values.size() >= index)
		return item_values[index];
	else return 0;
}
