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
#include "dungeon.hpp"

#include "ability.hpp"
#include "player.hpp"

AbilityManager::AbilityManager()
{

}
AbilityManager::~AbilityManager()
{
	free();
}
void AbilityManager::free()
{

}
void AbilityManager::init_abilities(Dungeon *scene)
{
	Ability *temp = new Ability;
	temp->init(scene, "core/script/ability/melee_attack.lua", SDLK_1);
	temp->set_ui_position(0, 0);
	player->add_ability(temp->get_name(), temp);

	/*temp = new Ability;
	temp->init(scene, "core/script/ability/summon_door.lua", SDLK_2);
	temp->set_ui_position(48, 0);
	player->add_ability(temp->get_name(), temp);

	temp = new Ability;
	temp->init(scene, "core/script/ability/call_mount.lua", SDLK_3);
	temp->set_ui_position(96, 0);
	player->add_ability(temp->get_name(), temp);

	temp = new Ability;
	temp->init(scene, "core/script/ability/fire_breath.lua", SDLK_4);
	temp->set_ui_position(144, 0);
	player->add_ability(temp->get_name(), temp);*/

	temp = new Ability;
	temp->init(scene, "core/script/ability/charge.lua", SDLK_2);
	temp->set_ui_position(48, 0);
	player->add_ability(temp->get_name(), temp);
}
void AbilityManager::render_abilities() const
{
	Ability *hovered = nullptr;
	for (auto &it : player->get_abilities())
	{
		if (it.second != nullptr && it.second->get_target_type() != AT_PASSIVE)
		{
			if (it.second->get_hovered())
				hovered = it.second;
			else it.second->render();
		}
	}
	if (hovered != nullptr) // If there's a hovered ability, render it last (to see the info box on top)
	{
		hovered->render();
		hovered = nullptr;
	}
}
bool AbilityManager::overlap_abilities(int16_t xpos, int16_t ypos) const
{
	bool overlap = false;
	for (auto &it : player->get_abilities())
	{
		if (it.second != nullptr)
		{
			// Don't overwrite the bool if there's no overlap
			if (it.second->get_overlap(xpos, ypos) && !overlap)
				overlap = true;
		}
	}
	return overlap;
}
bool AbilityManager::click_abilities() const
{
	for (auto &it : player->get_abilities())
	{
		// Abort immediately if an ability is clicked
		bool click = false;
		if (it.second != nullptr)
			click = it.second->get_click();
		if (click) return true;
	}
	return false;
}
void AbilityManager::trigger_abilities(uint8_t trigger)
{
	for (auto &it : player->get_abilities()) if (it.second != nullptr)
	{
		if (it.second->get_trigger_type() == trigger)
			it.second->activate();
	}
}
void AbilityManager::cooldown_abilities(uint8_t amount)
{
	for (auto &it : player->get_abilities()) if (it.second != nullptr)
		it.second->reduce_cooldown(amount);
}
bool AbilityManager::keyboard_input(SDL_Keycode key)
{
	for (auto &it : player->get_abilities()) if (it.second != nullptr)
	{
		if (it.second->get_hotkey() == key)
		{
			if (it.second->get_activated())
				it.second->cancel();
			else if (player->get_ability() == nullptr)
				it.second->activate();
			return true;
		}
		else if (key == SDLK_ESCAPE && it.second->get_activated())
		{
			it.second->cancel();
			return true;
		}
	}
	return false;
}
