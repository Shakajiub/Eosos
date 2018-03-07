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
#include "ability_manager.hpp"
#include "ability.hpp"
#include "hero.hpp"

#include "ability_sleep.hpp"
#include "camera.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

AbilityManager::AbilityManager()
{

}
AbilityManager::~AbilityManager()
{
	free();
}
void AbilityManager::free()
{
	for (uint8_t i = 0; i < abilities.size(); i++)
	{
		if (abilities[i] != nullptr)
			delete abilities[i];
	}
	abilities.clear();
}
void AbilityManager::render_ui(Hero *hero) const
{
	if (hero != nullptr)
	{
		const uint16_t xpos = camera.get_cam_w() - 48;
		uint16_t ypos = 48;

		for (auto *a : abilities)
		{
			if (hero->get_has_ability(a->get_ability_name()))
			{
				a->render(xpos, ypos);
				ypos += 48;
			}
		}
		ui.get_bitmap_font()->render_text(camera.get_cam_w() - 96, 16,
			"Moves: " + std::to_string(hero->get_moves().first) + "/" + std::to_string(hero->get_moves().second)
		);
	}
}
void AbilityManager::load_ability(const std::string &ability)
{
	for (auto *a : abilities)
	{
		if (a->get_ability_name() == ability)
			return;
	}
	Ability *new_ability = nullptr;
	if (ability == "sleep")
		new_ability = new AbilitySleep;

	if (new_ability != nullptr)
	{
		if (new_ability->init(ability))
			abilities.push_back(new_ability);
		else delete new_ability;
	}
}
bool AbilityManager::get_overlap(int16_t mouse_x, int16_t mouse_y) const
{
	const uint16_t xpos = camera.get_cam_w() - 48;
	uint16_t ypos = 48;
	bool overlap = false;

	for (Ability *a : abilities)
	{
		if (mouse_x > xpos && mouse_y > ypos && mouse_y < ypos + 48)
		{
			a->set_hovered(true);
			overlap = true;
		}
		else a->set_hovered(false);
		ypos += 48;
	}
	return overlap;
}
bool AbilityManager::get_click(Hero *hero, int16_t mouse_x, int16_t mouse_y) const
{
	const uint16_t xpos = camera.get_cam_w() - 48;
	uint16_t ypos = 48;

	for (Ability *a : abilities)
	{
		if (mouse_x > xpos && mouse_y > ypos && mouse_y < ypos + 48)
		{
			a->apply(hero);
			return true;
		}
		ypos += 48;
	}
	return false;
}
