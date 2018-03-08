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
