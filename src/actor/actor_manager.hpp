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

class Level;

class ActorManager
{
public:
	ActorManager();
	~ActorManager();

	void free();
	void update(Level *level);
	void render(Level *level);
	void animate();

	bool spawn_actor(Level *level, ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name);
	void input_keyboard_down(SDL_Keycode key);

private:
	Actor *current_actor;
};

#endif // ACTOR_MANAGER
