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

enum GameState
{
	GAME_IN_PROGRESS,
	GAME_OVER,
	GAME_BOSS_WON,
	GAME_END
};
class Scenario : public Scene
{
public:
	Scenario();
	~Scenario();

	virtual void free();
	virtual void init();

	virtual bool update();
	virtual void render() const;

	void next_turn();

	Level* get_level() const { return current_level; }

private:
	GameState state;

	uint8_t base_health;
	uint8_t anim_timer;
	uint8_t current_depth;

	Actor *hovered_actor;
	Level *current_level;

	Texture *node_highlight;
	Texture *base_healthbar;
	std::vector<Texture*> pointers;

	uint8_t frames;
	uint8_t display_fps;
	uint16_t frame_counter;

	int mouse_x, mouse_y; // for SDL_GetMouseState() from update() to render()
	int8_t dir_x, dir_y;
};

#endif // OVERWORLD_HPP
