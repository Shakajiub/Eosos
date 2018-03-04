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
#include "hero.hpp"

Hero::Hero()
{

}
Hero::~Hero()
{
	free();
}
void Hero::free()
{

}
void Hero::start_turn()
{
	turn_done = false;
	moves = std::make_pair(1, 1);
}
bool Hero::take_turn()
{
	if (turn_done)
	{
		moves.first -= 1;
		if (moves.first > 0)
			turn_done = false;
	}
	return Actor::take_turn();
}
void Hero::end_turn()
{
	Actor::end_turn();
}
void Hero::input_keyboard_down(SDL_Keycode key)
{
	int8_t offset_x = 0;
	int8_t offset_y = 0;
	switch (key)
	{
		case SDLK_UP: case SDLK_KP_8: case SDLK_k: offset_y = -1; break;
		case SDLK_DOWN: case SDLK_KP_2: case SDLK_j: offset_y = 1; break;
		case SDLK_LEFT: case SDLK_KP_4: case SDLK_h: offset_x = -1; break;
		case SDLK_RIGHT: case SDLK_KP_6: case SDLK_l: offset_x = 1; break;
		case SDLK_KP_7: case SDLK_y: offset_x = -1; offset_y = -1; break;
		case SDLK_KP_9: case SDLK_u: offset_x = 1; offset_y = -1; break;
		case SDLK_KP_1: case SDLK_b: offset_x = -1; offset_y = 1; break;
		case SDLK_KP_3: case SDLK_n: offset_x = 1; offset_y = 1; break;
		case SDLK_KP_5: case SDLK_SPACE: turn_done = true; break;
		default: break;
	}
	if (offset_x != 0 || offset_y != 0)
		add_action(ACTION_MOVE, grid_x + offset_x, grid_y + offset_y);
}
