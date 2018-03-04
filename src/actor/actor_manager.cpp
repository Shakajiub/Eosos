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
#include "actor_manager.hpp"
#include "level.hpp"

#include "hero.hpp"

ActorManager::ActorManager() : current_actor(nullptr)
{

}
ActorManager::~ActorManager()
{
	free();
}
void ActorManager::free()
{
	if (current_actor != nullptr)
	{
		delete current_actor;
		current_actor = nullptr;
	}
}
void ActorManager::update(Level *level)
{
	if (current_actor != nullptr)
	{
		while (current_actor->take_turn())
		{
			current_actor->end_turn();

			// TODO - Loop to the next actor

			current_actor->start_turn();
		}
		current_actor->update(level);
	}
}
void ActorManager::render(Level *level)
{
	Actor *temp_actor = nullptr;
	for (uint8_t y = 0; y < level->get_map_height(); y++)
	{
		for (uint8_t x = 0; x < level->get_map_width(); x++)
		{
			temp_actor = level->get_actor(x, y);
			if (temp_actor != nullptr)
				temp_actor->render();
		}
	}
}
void ActorManager::animate()
{
	if (current_actor != nullptr)
		current_actor->action_idle();
}
bool ActorManager::spawn_actor(Level *level, ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name)
{
	std::cout << "1" << std::endl;
	if (level != nullptr && !level->get_wall(xpos, ypos, true))
	{
		std::cout << "2" << std::endl;
		current_actor = new Hero;
		if (!current_actor->init(at, xpos, ypos, texture_name))
		{
			std::cout << "333" << std::endl;
			delete current_actor;
			current_actor = nullptr;
		}
		else level->set_actor(xpos, ypos, current_actor);
		std::cout << "4" << std::endl;
	}
}
void ActorManager::input_keyboard_down(SDL_Keycode key)
{
	if (current_actor != nullptr)
		current_actor->input_keyboard_down(key);
}
