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

#ifndef HERO_HPP
#define HERO_HPP

#include "actor.hpp"

class AStar;

class Hero : public Actor
{
public:
	Hero();
	~Hero();

	void free();

	virtual void render() const;
	virtual void start_turn();
	virtual bool take_turn(Level *level);
	virtual void end_turn();

	void init_pathfinder();
	void step_pathfinder(Level *level);

	void input_keyboard_down(SDL_Keycode key, Level *level);
	void input_mouse_button_down(SDL_Event eve, Level *level);

	AStar* get_pathfinder() const { return pathfinder; }
	bool get_auto_move() const;

private:
	bool auto_move_path;
	bool command_this_turn;

	AStar *pathfinder;
};

#endif // HERO_HPP
