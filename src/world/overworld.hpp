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

#ifndef OVERWORLD_HPP
#define OVERWORLD_HPP

#include "scene.hpp"

class ActorManager;
class Actor;
class Level;
class Texture;

class Overworld : public Scene
{
public:
	Overworld();
	~Overworld();

	virtual void free();
	virtual void init();

	virtual bool update();
	virtual void render() const;

	void next_turn();

	Level* get_level() const { return current_level; }

private:
	uint8_t anim_timer;
	uint8_t current_depth;

	ActorManager *actor_manager;
	Actor *hovered_actor;
	Level *current_level;

	Texture *node_highlight;
	std::vector<Texture*> pointers;

	uint8_t frames;
	uint8_t display_fps;
	uint16_t frame_counter;

	int mouse_x, mouse_y; // for SDL_GetMouseState() from update() to render()
};

#endif // OVERWORLD_HPP
