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

#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <queue>

class Level;
class Texture;

enum ActorType
{
	ACTOR_NULL,
	ACTOR_HERO,
	ACTOR_MONSTER
};
enum ActionType
{
	ACTION_NULL,
	ACTION_MOVE,
	ACTION_ATTACK
};
typedef struct
{
	ActionType type;
	uint8_t xpos, ypos;
}
Action;

class Actor
{
public:
	Actor();
	~Actor();

	void free();
	bool init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name);

	virtual void update(Level *level);
	virtual void render() const;

	virtual void start_turn();
	virtual bool take_turn();
	virtual void end_turn();

	void add_action(ActionType at, uint8_t xpos, uint8_t ypos);
	void action_idle();
	bool action_move(Level *level);
	bool action_attack();

	void load_bubble(const std::string &bubble_name, uint8_t timer = 0);
	void render_bubble() const;

	ActorType get_actor_type() const { return actor_type; }
	uint16_t get_ID() const { return actor_ID; }
	uint8_t get_grid_x() const { return grid_x; }
	uint8_t get_grid_y() const { return grid_y; }
	std::pair<int8_t, int8_t> get_moves() const { return moves; }

protected:
	bool in_camera;
	bool turn_done;
	bool facing_right;

	ActorType actor_type;
	uint16_t actor_ID;
	uint16_t x, y;
	uint8_t grid_x, grid_y;
	uint8_t prev_x, prev_y;

	uint8_t anim_frames;
	uint8_t anim_timer;

	Action current_action;
	std::queue<Action> action_queue;
	std::pair<int8_t, int8_t> moves;

	uint8_t bubble_timer;
	Texture *bubble;

	SDL_Rect frame_rect;
	Texture *texture;

	static uint16_t ID;
};

#endif // ACTOR_HPP
