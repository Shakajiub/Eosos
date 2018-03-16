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
#include "ability_dismount.hpp"
#include "hero.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "scene_manager.hpp"
#include "texture_manager.hpp"
#include "level.hpp"
#include "scenario.hpp"

AbilityDismount::AbilityDismount() : target_texture(nullptr), temp_hero(nullptr)
{
	valid_nodes.clear();
	ability_desc = "Dismount";
}
AbilityDismount::~AbilityDismount()
{
	free();
}
void AbilityDismount::free()
{
	if (target_texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(target_texture->get_name());
		target_texture = nullptr;
	}
	valid_nodes.clear();
}
bool AbilityDismount::init()
{
	if (init_texture("ui/icon/horse.png", DAWN_EARTH))
		ability_name = "dismount";

	target_texture = engine.get_texture_manager()->load_texture("ui/target.png", true);
	if (target_texture != nullptr)
	{
		target_texture->set_color(DAWN_BERRY);
		return true;
	}
	return false;
}
void AbilityDismount::apply(Hero *hero, bool cancel)
{
	if (!activated && !cancel)
	{
		Scenario *scene = engine.get_scene_manager()->get_scene("scenario");
		Level *level = nullptr;

		if (scene != nullptr)
			level = scene->get_level();

		if (level == nullptr || hero == nullptr)
			return;

		valid_nodes.clear();

		hero->set_ability_activated(true);
		hero->clear_pathfinder();

		const uint8_t xpos = hero->get_grid_x();
		const uint8_t ypos = hero->get_grid_y();

		if (!level->get_wall(xpos, ypos - 1, true)) valid_nodes.push_back(std::make_pair(xpos, ypos - 1));
		if (!level->get_wall(xpos, ypos + 1, true)) valid_nodes.push_back(std::make_pair(xpos, ypos + 1));
		if (!level->get_wall(xpos - 1, ypos, true)) valid_nodes.push_back(std::make_pair(xpos - 1, ypos));
		if (!level->get_wall(xpos + 1, ypos, true)) valid_nodes.push_back(std::make_pair(xpos + 1, ypos));

		activated = true;
		temp_hero = hero;
	}
	else clear(hero);
}
void AbilityDismount::render(uint16_t xpos, uint16_t ypos, SDL_Keycode key) const
{
	Ability::render(xpos, ypos, key);

	if (target_texture != nullptr) for (auto node : valid_nodes)
		target_texture->render(node.first * 32 - camera.get_cam_x(), node.second * 32 - camera.get_cam_y());
}
bool AbilityDismount::get_click(uint16_t mouse_x, uint16_t mouse_y)
{
	const int8_t map_x = (mouse_x + camera.get_cam_x()) / 32;
	const int8_t map_y = (mouse_y + camera.get_cam_y()) / 32;

	for (auto node : valid_nodes)
	{
		if (node.first == map_x && node.second == map_y)
		{
			if (temp_hero != nullptr)
			{
				temp_hero->remove_ability("dismount");
				temp_hero->add_action(ACTION_MOVE, map_x, map_y, 1);
				temp_hero->set_moves(0);
			}
			clear(temp_hero);
			return true;
		}
	}
	return activated;
}
void AbilityDismount::clear(Hero *hero)
{
	valid_nodes.clear();

	if (hero != nullptr)
		hero->set_ability_activated(false);

	activated = false;
	temp_hero = nullptr;
}
