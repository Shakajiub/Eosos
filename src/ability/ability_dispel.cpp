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
#include "ability_dispel.hpp"
#include "hero.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "scene_manager.hpp"
#include "texture_manager.hpp"
#include "level.hpp"
#include "scenario.hpp"

AbilityDispel::AbilityDispel() : target_texture(nullptr), temp_hero(nullptr)
{
	valid_nodes.clear();
	ability_desc = "Dispel status from nearby creature";
}
AbilityDispel::~AbilityDispel()
{
	free();
}
void AbilityDispel::free()
{
	if (target_texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(target_texture->get_name());
		target_texture = nullptr;
	}
	valid_nodes.clear();
}
bool AbilityDispel::init()
{
	if (init_texture("ui/icon/dispel.png", DAWN_CORNFLOWER))
		ability_name = "dispel";

	target_texture = engine.get_texture_manager()->load_texture("ui/target.png", true);
	if (target_texture != nullptr)
	{
		target_texture->set_color(DAWN_BERRY);
		return true;
	}
	return false;
}
void AbilityDispel::apply(Hero *hero, bool cancel)
{
	if (!activated && !cancel)
	{
		Scenario *scene = engine.get_scene_manager()->get_scene("scenario");
		Level *level = nullptr;

		if (scene != nullptr)
			level = scene->get_level();

		if (level == nullptr)
			return;

		const uint8_t x = hero->get_grid_x();
		const uint8_t y = hero->get_grid_y();

		for (int8_t ypos = -2; ypos < 3; ypos++)
		{
			for (int8_t xpos = -2; xpos < 3; xpos++)
			{
				Actor *temp_actor = level->get_actor(x + xpos, y + ypos);
				if (temp_actor != nullptr)
				{
					TargetNode tn;
					tn.xpos = x + xpos;
					tn.ypos = y + ypos;
					tn.target = temp_actor;
					valid_nodes.push_back(tn);
				}
			}
		}
		hero->clear_pathfinder();
		hero->set_ability_activated(true);

		activated = true;
		temp_hero = hero;
	}
	else clear(hero);
}
void AbilityDispel::render(uint16_t xpos, uint16_t ypos, SDL_Keycode key) const
{
	Ability::render(xpos, ypos, key);

	if (target_texture != nullptr) for (auto node : valid_nodes)
		target_texture->render(node.xpos * 32 - camera.get_cam_x(), node.ypos * 32 - camera.get_cam_y());
}
bool AbilityDispel::get_click(uint16_t mouse_x, uint16_t mouse_y)
{
	const int8_t map_x = (mouse_x + camera.get_cam_x()) / 32;
	const int8_t map_y = (mouse_y + camera.get_cam_y()) / 32;

	for (auto node : valid_nodes)
	{
		if (node.xpos == map_x && node.ypos == map_y)
		{
			if (temp_hero != nullptr)
			{
				temp_hero->add_action(ACTION_INTERACT, temp_hero->get_grid_x(), temp_hero->get_grid_y());
				temp_hero->set_moves(0);
			}
			if (node.target != nullptr)
			{
				node.target->set_status(STATUS_NONE);
			}
			clear(temp_hero);
			return true;
		}
	}
	return activated;
}
void AbilityDispel::clear(Hero *hero)
{
	valid_nodes.clear();

	if (hero != nullptr)
		hero->set_ability_activated(false);

	activated = false;
	temp_hero = nullptr;
}
