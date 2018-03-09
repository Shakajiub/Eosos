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
#include "mount.hpp"
#include "level.hpp"

Mount::Mount() : rider(nullptr)
{

}
Mount::~Mount()
{
	free();
}
void Mount::free()
{
	if (rider != nullptr)
	{
		rider->set_mount(nullptr);
		rider = nullptr;
	}
}
void Mount::update(Level *level)
{
	Actor::update(level);

	if (rider != nullptr)
	{
		if (x != rider->get_x() || y != rider->get_y())
		{
			facing_right = rider->get_facing_right();
			x = rider->get_x(); y = rider->get_y();
		}
		frame_rect.y = rider->get_frame_rect().y;
		in_camera = rider->get_in_camera();
	}
}
void Mount::start_turn()
{
	if (rider == nullptr)
	{
		turn_done = false;
		moves = std::make_pair(1, 1);
	}
	else turn_done = true;
}
bool Mount::take_turn(Level *level, ActorManager *am)
{
	if (turn_done)
		return true;

	if (actions_empty() && moves.first > 0)
	{
		if (engine.get_rng() % 10 == 0)
		{
			const int8_t offset_x[4] = { 0, 0, -1, 1 };
			const int8_t offset_y[4] = { -1, 1, 0, 0 };
			const uint8_t i = engine.get_rng() % 4;

			if (!level->get_wall(grid_x + offset_x[i], grid_y + offset_y[i], true))
				add_action(ACTION_MOVE, grid_x + offset_x[i], grid_y + offset_y[i]);
			else turn_done = true;
			moves.first = 0;
		}
		else turn_done = true;
	}
	return turn_done;
}
void Mount::set_rider(Actor *new_rider)
{
	rider = new_rider;
	if (rider != nullptr)
	{
		x = rider->get_x(); y = rider->get_y();
		facing_right = rider->get_facing_right();
		frame_rect.y = rider->get_frame_rect().y;
		in_camera = rider->get_in_camera();
	}
}
