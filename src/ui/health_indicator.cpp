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
#include "health_indicator.hpp"
#include "actor.hpp"
#include "texture.hpp"

#include "player.hpp"
#include "texture_manager.hpp"

HealthIndicator::HealthIndicator() : shake(0), heart_sheet(nullptr), target(nullptr)
{
	health = std::make_pair(0, 0);
}
HealthIndicator::~HealthIndicator()
{
	free();
}
void HealthIndicator::free()
{
	if (heart_sheet != nullptr)
		engine.get_texture_manager()->free_texture(heart_sheet->get_name());
}
void HealthIndicator::init(Actor *new_target, const std::string &heart_texture)
{
	if (new_target != nullptr)
	{
		target = new_target;
		health = target->get_health();
	}
	heart_sheet = engine.get_texture_manager()->load_texture(heart_texture);
}
void HealthIndicator::update()
{
	if (shake > 0) shake -= 1;
	if (target != nullptr)
	{
		const int8_t prev_hp = health.first;
		health = target->get_health();

		if (health.first < 1 && target != player)
		{
			health = std::make_pair(0, 0);
			target = nullptr;
		}
		else if (prev_hp > health.first)
			shake = 10;
	}
}
void HealthIndicator::render() const
{
	if (heart_sheet != nullptr && health.second > 0)
	{
		SDL_Rect rect = { 0, 0, 16, 16 };
		int8_t hearts = health.second / 3;
		int8_t hp_left = health.first;
		uint16_t render_x = x;

		while (hearts > 0)
		{
			if (hp_left == 2) rect.x = 16;
			else if (hp_left == 1) rect.x = 32;
			else if (hp_left < 1) rect.x = 48;

			if (shake > 0)
				heart_sheet->render(render_x, y + (engine.get_rng() % shake), &rect);
			else heart_sheet->render(render_x, y, &rect);

			hearts -= 1;
			hp_left -= 3;
			render_x += 32;
		}
	}
}
