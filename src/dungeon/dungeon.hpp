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

#ifndef DUNGEON_HPP
#define DUNGEON_HPP

#include "scene.hpp"
#include <vector>

class Actor;
class Level;
class Texture;

class Dungeon : public Scene
{
public:
	Dungeon();
	~Dungeon();

	virtual void free();
	virtual void init();

	virtual bool update();
	virtual void render() const;

	void finish_level() { regen_level = true; }

	Actor* get_current_actor() const { return current_actor; }
	Level* get_level() const { return current_level; }

private:
	bool regen_level;
	uint8_t anim_timer;

	Actor *current_actor;
	Level *current_level;
	Texture *node_highlight;
	std::vector<Texture*> pointers;

	uint8_t frames, display_fps;
	uint16_t frame_counter;
};

#endif // DUNGEON_HPP
