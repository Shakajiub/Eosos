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

#ifndef ACTOR_MANAGER
#define ACTOR_MANAGER

#include "actor.hpp"

#include <vector>

class AbilityManager;
class Level;

class ActorManager
{
public:
	ActorManager();
	~ActorManager();

	void free();
	void init();
	bool update(Level *level);
	void render(Level *level) const;
	void animate();

	void render_ui() const;

	void clear_actors(Level *level, bool clear_heroes = false);
	Actor* spawn_actor(Level *level, ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name);
	void place_actors(Level *level, std::pair<uint8_t, uint8_t> base_pos);

	void input_keyboard_down(SDL_Keycode key, Level *level);
	void input_mouse_button_down(SDL_Event eve, Level *level);

	bool get_next_turn();

	bool get_overlap(int16_t mouse_x, int16_t mouse_y) const;
	bool get_click(int16_t mouse_x, int16_t mouse_y) const;

private:
	std::pair<uint8_t, uint8_t> find_spot(Level *level, uint8_t xpos, uint8_t ypos) const;
	void delete_actor(Level *level, Actor *actor);

	bool next_turn;
	Actor *current_actor;
	std::vector<Actor*> actors;
	std::vector<Actor*> heroes;
	AbilityManager *ability_manager;
};

#endif // ACTOR_MANAGER
