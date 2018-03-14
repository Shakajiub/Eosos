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
#include "ability_shoot.hpp"
#include "hero.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "scene_manager.hpp"
#include "texture_manager.hpp"
#include "level.hpp"
#include "overworld.hpp"

AbilityShoot::AbilityShoot() : target_texture(nullptr), temp_hero(nullptr)
{
	valid_nodes.clear();
	ability_desc = "Ranged attack";
}
AbilityShoot::~AbilityShoot()
{
	free();
}
void AbilityShoot::free()
{
	if (target_texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(target_texture->get_name());
		target_texture = nullptr;
	}
	valid_nodes.clear();
}
bool AbilityShoot::init()
{
	if (init_texture("core/texture/ui/icon/bow.png", DAWN_EARTH))
		ability_name = "shoot";

	target_texture = engine.get_texture_manager()->load_texture("core/texture/ui/target.png", true);
	if (target_texture != nullptr)
	{
		target_texture->set_color(DAWN_BERRY);
		return true;
	}
	return false;
}
void AbilityShoot::apply(Hero *hero, bool cancel)
{
	if (!activated && !cancel)
	{
		Overworld *scene = engine.get_scene_manager()->get_scene("test");
		Level *level = nullptr;

		if (scene != nullptr)
			level = scene->get_level();

		if (level == nullptr)
			return;

		const int8_t offset_x[12] = { -1, 0, 1, -2, -2, -2, 2, 2, 2, -1, 0, 1 };
		const int8_t offset_y[12] = { -2, -2, -2, -1, 0, 1, -1, 0, 1, 2, 2, 2 };
		valid_nodes.clear();

		//   xxx
		//  x...x    x = valid positions included in (offset_x[i], offset_y[i])
		//  x.@.x    . = FoV blocking positions (just change any 2 into 1 in the offsets)
		//  x...x    @ = hero position
		//   xxx

		const uint8_t x = hero->get_grid_x();
		const uint8_t y = hero->get_grid_y();

		for (uint8_t i = 0; i < 12; i++)
		{
			uint8_t block_x = x + offset_x[i];
			if (offset_x[i] == -2) block_x += 1;
			else if (offset_x[i] == 2) block_x -= 1;

			uint8_t block_y = y + offset_y[i];
			if (offset_y[i] == -2) block_y += 1;
			else if (offset_y[i] == 2) block_y -= 1;

			if (!level->get_wall(block_x, block_y) && !level->get_wall(x + offset_x[i], y + offset_y[i]))
				valid_nodes.push_back(std::make_pair(x + offset_x[i], y + offset_y[i]));
		}
		hero->clear_pathfinder();
		hero->set_ability_activated(true);

		activated = true;
		temp_hero = hero;
	}
	else clear(hero);
}
void AbilityShoot::render(uint16_t xpos, uint16_t ypos, SDL_Keycode key) const
{
	Ability::render(xpos, ypos, key);

	if (target_texture != nullptr) for (auto node : valid_nodes)
		target_texture->render(node.first * 32 - camera.get_cam_x(), node.second * 32 - camera.get_cam_y());
}
bool AbilityShoot::get_click(uint16_t mouse_x, uint16_t mouse_y)
{
	const int8_t map_x = (mouse_x + camera.get_cam_x()) / 32;
	const int8_t map_y = (mouse_y + camera.get_cam_y()) / 32;

	for (auto node : valid_nodes)
	{
		if (node.first == map_x && node.second == map_y)
		{
			if (temp_hero != nullptr)
			{
				temp_hero->add_action(ACTION_SHOOT, map_x, map_y);
				temp_hero->set_moves(0);
			}
			clear(temp_hero);
			return true;
		}
	}
	return activated;
}
void AbilityShoot::clear(Hero *hero)
{
	valid_nodes.clear();

	if (hero != nullptr)
		hero->set_ability_activated(false);

	activated = false;
	temp_hero = nullptr;
}
