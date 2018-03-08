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

#include "ability_dismount.hpp"
#include "ability_levelup.hpp"
#include "ability_shoot.hpp"
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
		const SDL_Keycode hotkeys[10] = {
			SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_0
		};
		const uint16_t xpos = camera.get_cam_w() - 48;
		uint16_t ypos = 48;

		for (auto *a : abilities)
		{
			if (hero->has_ability(a->get_ability_name()))
			{
				a->render(xpos, ypos, hotkeys[ypos / 48 - 1]);
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
	else if (ability == "shoot")
		new_ability = new AbilityShoot;
	else if (ability == "level-up")
		new_ability = new AbilityLevelUp;
	else if (ability == "dismount")
		new_ability = new AbilityDismount;

	if (new_ability != nullptr)
	{
		if (new_ability->init())
			abilities.push_back(new_ability);
		else delete new_ability;
	}
}
void AbilityManager::clear()
{
	for (Ability *a : abilities)
		a->clear();
}
void AbilityManager::input_keyboard_down(Hero *hero, SDL_Keycode key)
{
	const SDL_Keycode hotkeys[10] = {
		SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_0
	};
	uint8_t i = 0;
	for (auto *a : abilities)
	{
		if (hero->has_ability(a->get_ability_name()))
		{
			if (key == hotkeys[i])
			{
				a->apply(hero);
				return;
			}
			i += 1;
		}
	}
}
bool AbilityManager::get_overlap(Hero *hero, int16_t mouse_x, int16_t mouse_y) const
{
	const uint16_t xpos = camera.get_cam_w() - 48;
	uint16_t ypos = 48;
	bool overlap = false;

	for (Ability *a : abilities)
	{
		if (!hero->has_ability(a->get_ability_name()))
			continue;

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
		if (!hero->has_ability(a->get_ability_name()))
			continue;

		if (mouse_x > xpos && mouse_y > ypos && mouse_y < ypos + 48)
		{
			a->apply(hero);
			return true;
		}
		else if (a->get_click(mouse_x, mouse_y))
			return true;

		ypos += 48;
	}
	return false;
}
