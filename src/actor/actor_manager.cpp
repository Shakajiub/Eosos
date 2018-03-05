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
#include "monster.hpp"
#include "camera.hpp"

#include <algorithm> // for std::find

ActorManager::ActorManager() : next_turn(false), current_actor(nullptr)
{

}
ActorManager::~ActorManager()
{
	free();
}
void ActorManager::free()
{
	for (uint8_t i = 0; i < actors.size(); i++)
	{
		if (actors[i] != nullptr)
			delete actors[i];
	}
	actors.clear();
	current_actor = nullptr;
}
void ActorManager::update(Level *level)
{
	if (current_actor != nullptr)
	{
		while (current_actor->take_turn(level))
		{
			current_actor->end_turn();

			const uint16_t current_ID = current_actor->get_ID();
			Actor *temp_actor = nullptr;
			Actor *first_actor = nullptr;

			std::vector<Actor*> to_erase;
			for (Actor *a : actors)
			{
				if (a->get_delete())
				{
					to_erase.push_back(a);
					continue;
				}
				if (a->get_ID() > current_ID && (temp_actor == nullptr || a->get_ID() < temp_actor->get_ID()))
					temp_actor = a;
				if (first_actor == nullptr || a->get_ID() < first_actor->get_ID())
					first_actor = a;
			}
			if (to_erase.size() > 0) for (Actor *a : to_erase)
			{
				std::vector<Actor*>::iterator pos = std::find(actors.begin(), actors.end(), a);
				if (pos != actors.end())
					actors.erase(pos);
				level->set_actor(a->get_grid_x(), a->get_grid_y(), nullptr);
				delete a;
			}
			if (temp_actor == nullptr)
			{
				current_actor = first_actor;
				next_turn = true;
			}
			else current_actor = temp_actor;
			current_actor->start_turn();

			if (current_actor->get_actor_type() == ACTOR_HERO)
				camera.update_position(current_actor->get_grid_x() * 32, current_actor->get_grid_y() * 32);
		}
		for (Actor * a : actors)
			a->update(level);
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
	for (Actor *a : actors)
	{
		if (a->get_actor_type() != ACTOR_HERO || a == current_actor)
			a->action_idle();
	}
}
void ActorManager::clear_actors()
{
	for (uint8_t i = 0; i < actors.size(); i++)
	{
		if (actors[i] != nullptr)
			delete actors[i];
	}
	actors.clear();
	current_actor = nullptr;
}
bool ActorManager::spawn_actor(Level *level, ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name)
{
	if (level != nullptr && !level->get_wall(xpos, ypos, true))
	{
		Actor *temp = nullptr;

		if (at == ACTOR_HERO)
			temp = new Hero;
		else if (at == ACTOR_MONSTER)
			temp = new Monster;

		if (temp != nullptr)
		{
			if (temp->init(at, xpos, ypos, texture_name))
			{
				actors.push_back(temp);
				if (current_actor == nullptr)
				{
					current_actor = temp;
					current_actor->start_turn();
				}
				level->set_actor(xpos, ypos, temp);

				if (temp->get_actor_type() == ACTOR_HERO)
					camera.update_position(temp->get_grid_x() * 32, temp->get_grid_y() * 32);
			}
			else delete temp;
		}
	}
}
void ActorManager::input_keyboard_down(SDL_Keycode key, Level *level)
{
	if (current_actor != nullptr && current_actor->get_actor_type() == ACTOR_HERO)
		dynamic_cast<Hero*>(current_actor)->input_keyboard_down(key, level);
}
bool ActorManager::get_next_turn()
{
	if (next_turn)
	{
		next_turn = false;
		return true;
	}
	return false;
}
