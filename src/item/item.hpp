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

#ifndef ITEM_HPP
#define ITEM_HPP

#include <vector>

class Texture;

enum EquipType
{
	ET_NONE, ET_AMMO, ET_AMULET, ET_ARMOR,
	ET_BOOTS, ET_CLOAK, ET_GLOVES, ET_HAT,
	ET_LIGHT, ET_SHIELD, ET_TOOL, ET_RING,
	ET_WEAPON
};
enum MiscType
{
	MT_NONE, MT_BOOK, MT_FOOD, MT_GEM,
	MT_MATERIAL, MT_MONEY, MT_POTION, MT_SCROLL
};
enum WeaponType
{
	WT_NONE, WT_AXE, WT_DAGGER, WT_HAMMER,
	WT_POLEARM, WT_RANGED, WT_SHIELD, WT_SWORD,
	WT_WAND
};
enum ItemSize
{
	IS_NONE, IS_SMALL, IS_MEDIUM, IS_LARGE
};
enum DamageType // TODO - This should not be defined in here
{
	DT_NONE, DT_BLUNT, DT_PIERCE, DT_SLASH
};
class Item
{
public:
	Item();
	~Item();

	void free();
	bool init(const std::string &script_name, const std::string &item_name, bool free_script = true);

	void render(int16_t xpos, int16_t ypos, bool info = false) const;
	void render_info(int16_t xpos, int16_t ypos) const;

	bool get_in_use() const { return item_in_use; }
	uint8_t get_value(uint8_t index) const;
	uint8_t get_durability() const { return durability.first; }

	EquipType get_equip_type() const { return equip_type; }
	MiscType get_misc_type() const { return misc_type; }
	WeaponType get_weapon_type() const { return weapon_type; }
	ItemSize get_item_size() const { return item_size; }
	DamageType get_damage_type() const { return damage_type; }
	Texture *get_texture() const { return item_texture; }

	void set_in_use(bool in_use) { item_in_use = in_use; }
	void set_durability(uint8_t new_value) { durability.first = new_value; }

protected:
	bool item_in_use;

	EquipType equip_type;
	MiscType misc_type;
	WeaponType weapon_type;
	ItemSize item_size;
	DamageType damage_type;

	std::string item_name;
	std::string item_script;

	std::vector<uint8_t> item_values;
	std::pair<uint8_t, uint8_t> durability;

	Texture *item_texture;
	SDL_Texture *info_texture;
};

#endif // ITEM_HPP
