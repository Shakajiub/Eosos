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
#include "ability_manager.hpp"
#include "level.hpp"

#include "hero.hpp"
#include "monster.hpp"
#include "mount.hpp"
#include "prop.hpp"
#include "camera.hpp"

#include <algorithm> // for std::find & delete_actor()

ActorManager::ActorManager() : next_turn(false), current_actor(nullptr), ability_manager(nullptr)
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
	if (ability_manager != nullptr)
	{
		delete ability_manager;
		ability_manager = nullptr;
	}
	actors.clear();
	heroes.clear();
	current_actor = nullptr;
}
void ActorManager::init()
{
	ability_manager = new AbilityManager;
	ability_manager->load_ability("sleep");
	ability_manager->load_ability("shoot");
	ability_manager->load_ability("dispel");
	ability_manager->load_ability("sprout");
	ability_manager->load_ability("poison");
	ability_manager->load_ability("dismount");
	ability_manager->load_ability("level-up");
}
bool ActorManager::update(Level *level)
{
	bool actors_deleted = false;
	if (current_actor != nullptr && !next_turn)
	{
		Actor *prev_actor = current_actor;
		while (current_actor->take_turn(level))
		{
			current_actor->end_turn();

			if (current_actor->get_actor_type() == ACTOR_HERO)
				ability_manager->clear(dynamic_cast<Hero*>(current_actor));

			const uint16_t current_ID = current_actor->get_ID();
			Actor *temp_actor = nullptr;
			Actor *first_actor = nullptr;

			std::vector<Actor*> to_erase;
			for (Actor *a : actors)
			{
				if (a->get_delete())
					to_erase.push_back(a);
			}
			for (Actor *a : to_erase)
			{
				delete_actor(level, a);
				actors_deleted = true;
			}
			for (uint8_t i = 0; i < actors.size(); i++) if (actors[i] != nullptr)
			{
				if (actors[i]->get_ID() > current_ID && (temp_actor == nullptr || actors[i]->get_ID() < temp_actor->get_ID()))
					temp_actor = actors[i];
				if (first_actor == nullptr || actors[i]->get_ID() < first_actor->get_ID())
					first_actor = actors[i];
			}
			if (temp_actor == nullptr)
			{
				current_actor = first_actor;
				next_turn = true;
			}
			else current_actor = temp_actor;

			if (current_actor != nullptr)
				current_actor->start_turn();

			if (heroes.size() == 0 || current_actor == prev_actor)
				break;
		}
		for (Actor * a : actors)
			a->update(level);
	}
	return actors_deleted;
}
void ActorManager::render(Level *level) const
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
		a->action_idle();
}
void ActorManager::render_ui() const
{
	const uint16_t xpos = 0;
	uint16_t ypos = 48;

	for (Actor *a : actors)
	{
		if (a->get_actor_type() == ACTOR_HERO)
		{
			dynamic_cast<Hero*>(a)->render_ui(xpos, ypos);
			ypos += 48;

			if (ability_manager != nullptr && (a == current_actor || heroes.size() == 1))
				ability_manager->render_ui(dynamic_cast<Hero*>(a));
		}
		else if (a->get_actor_type() == ACTOR_MONSTER)
			a->render_ui(0, 0);
	}
}
void ActorManager::clear_actors(Level *level, bool clear_heroes)
{
	std::vector<Actor*> to_erase;
	for (Actor *a : actors)
	{
		if (!clear_heroes && a->get_actor_type() == ACTOR_HERO)
		{
			level->set_actor(a->get_grid_x(), a->get_grid_y(), nullptr);
			a->clear_mount();
		}
		else to_erase.push_back(a);
	}
	for (Actor *a : to_erase)
		delete_actor(level, a);

	if (clear_heroes)
	{
		actors.clear();
		heroes.clear();
		current_actor = nullptr;
	}
}
void ActorManager::clear_heroes(Level *level)
{
	std::vector<Actor*> to_erase;
	for (Actor *a : heroes)
		to_erase.push_back(a);

	for (Actor *a : to_erase)
		delete_actor(level, a);

	heroes.clear();
	//current_actor = nullptr;
}
Actor* ActorManager::spawn_actor(Level *level, ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name, bool place)
{
	if (level == nullptr)
		return nullptr;

	Point spot = find_spot(level, Point(xpos, ypos));
	if (spot.x != 0)
	{
		xpos = spot.x;
		ypos = spot.y;

		Actor *temp = nullptr;
		switch (at)
		{
			case ACTOR_HERO: temp = new Hero; break;
			case ACTOR_MONSTER: temp = new Monster; break;
			case ACTOR_MOUNT: temp = new Mount; break;
			case ACTOR_PROP: temp = new Prop; break;
			default: break;
		}
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
				if (place)
					level->set_actor(xpos, ypos, temp);

				if (temp->get_actor_type() == ACTOR_HERO)
					heroes.push_back(temp);
			}
			else
			{
				delete temp;
				temp = nullptr;
			}
		}
		return temp;
	}
	return nullptr;
}
void ActorManager::place_actors(Level *level, std::pair<uint8_t, uint8_t> base_pos)
{
	std::vector<Actor*> to_erase;
	for (Actor *a : actors)
	{
		Point spot = Point(0, 0);
		if (a->get_actor_type() == ACTOR_HERO)
		{
			spot = find_spot(level, Point(base_pos.first, base_pos.second));
			Hero *herp = dynamic_cast<Hero*>(a);

			herp->clear_status();
			herp->clear_pathfinder();
			herp->reset_moves();
		}
		else spot = find_spot(level, Point(a->get_grid_x(), a->get_grid_y()));

		if (spot.x != 0)
			level->set_actor(spot.x, spot.y, a);
		else to_erase.push_back(a);
	}
	for (Actor *a : to_erase)
		delete_actor(level, a);
}
void ActorManager::input_keyboard_down(SDL_Keycode key, Level *level)
{
	if (current_actor != nullptr && current_actor->get_actor_type() == ACTOR_HERO)
	{
		switch (key)
		{
			case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
			case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9: case SDLK_0:
				if (ability_manager != nullptr)
					ability_manager->input_keyboard_down(dynamic_cast<Hero*>(current_actor), key);
				break;
			default:
				dynamic_cast<Hero*>(current_actor)->input_keyboard_down(key, level);
				break;
		}
	}
}
void ActorManager::input_mouse_button_down(SDL_Event eve, Level *level)
{
	if (current_actor != nullptr && current_actor->get_actor_type() == ACTOR_HERO)
		dynamic_cast<Hero*>(current_actor)->input_mouse_button_down(eve, level);
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
bool ActorManager::get_overlap(int16_t mouse_x, int16_t mouse_y) const
{
	uint16_t ypos = 48;
	bool overlap = false;

	for (Actor *a : heroes)
	{
		if (mouse_x < 48 && mouse_y > ypos && mouse_y < ypos + 48)
		{
			a->set_hovered(HOVER_UI);
			overlap = true;
		}
		else if (a->get_hovered() != HOVER_MAP)
			a->set_hovered(HOVER_NONE);
		ypos += 48;
	}
	if (!overlap && ability_manager != nullptr && current_actor != nullptr && current_actor->get_actor_type() == ACTOR_HERO)
		return ability_manager->get_overlap(dynamic_cast<Hero*>(current_actor), mouse_x, mouse_y);
	return overlap;
}
bool ActorManager::get_click(int16_t mouse_x, int16_t mouse_y) const
{
	uint16_t ypos = 48;
	for (Actor *a : heroes)
	{
		if (mouse_x < 48 && mouse_y > ypos && mouse_y < ypos + 48)
		{
			camera.update_position(a->get_grid_x() * 32, a->get_grid_y() * 32);
			dynamic_cast<Hero*>(a)->set_sleep_timer(0);
			return true;
		}
		ypos += 48;
	}
	if (ability_manager != nullptr && current_actor != nullptr && current_actor->get_actor_type() == ACTOR_HERO)
		return ability_manager->get_click(dynamic_cast<Hero*>(current_actor), mouse_x, mouse_y);
	return false;
}
Point ActorManager::find_spot(Level *level, Point pos) const
{
	if (level->get_wall(pos.x, pos.y, true))
	{
		for (int8_t x = -1; x < 2; x++)
		{
			for (int8_t y = -1; y < 2; y++)
			{
				if (!level->get_wall(pos.x + x, pos.y + y, true))
					return Point(pos.x + x, pos.y + y);
			}
		}
		return Point(0, 0);
	}
	return pos;
}
void ActorManager::delete_actor(Level *level, Actor *actor)
{
	if (level == nullptr || actor == nullptr)
		return;

	bool victory = false;
	if (actor->get_actor_type() == ACTOR_MONSTER)
	{
		MonsterClass mc = dynamic_cast<Monster*>(actor)->get_monster_class();
		if (mc == MONSTER_PEST_SCORPION || mc == MONSTER_KOBOLD_TRUEFORM ||
			mc == MONSTER_DWARF_KING || mc == MONSTER_PLATINO)
			level->set_victory(true);
	}
	Mount *temp_mount = actor->get_mount();
	if (temp_mount != nullptr)
	{
		actor->clear_mount();
		level->set_actor(actor->get_grid_x(), actor->get_grid_y(), temp_mount);
	}
	else level->set_actor(actor->get_grid_x(), actor->get_grid_y(), nullptr);

	actors.erase(std::remove(actors.begin(), actors.end(), actor), actors.end());
	heroes.erase(std::remove(heroes.begin(), heroes.end(), actor), heroes.end());

	if (actor->get_actor_type() == ACTOR_HERO && heroes.size() == 0)
		level->set_damage_base(20);

	actor->death(level);
	delete actor;
}
