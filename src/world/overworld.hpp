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

private:
	uint8_t anim_timer;

	Actor *current_actor;
	Level *current_level;

	Texture *node_highlight;
	std::vector<Texture*> pointers;

	uint8_t frames, display_fps;
	uint16_t frame_counter;
};

#endif // OVERWORLD_HPP
