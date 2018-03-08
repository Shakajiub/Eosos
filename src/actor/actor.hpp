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
#include <vector>

class Mount;
class Level;
class Texture;

enum ActorType
{
	ACTOR_NULL,
	ACTOR_HERO,
	ACTOR_MONSTER,
	ACTOR_MOUNT,
	ACTOR_PROP
};
enum ActionType
{
	ACTION_NULL,
	ACTION_MOVE,
	ACTION_ATTACK,
	ACTION_SHOOT,
	ACTION_INTERACT
};
enum HoverType
{
	HOVER_NONE,
	HOVER_MAP,
	HOVER_UI
};
enum StatusType
{
	STATUS_NONE,
	STATUS_LEVELUP
};
typedef struct
{
	ActionType type;
	uint8_t xpos, ypos;
	int8_t action_value;
}
Action;

class Actor
{
public:
	Actor();
	~Actor();

	void free();
	virtual bool init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name);

	virtual void update(Level *level);
	virtual void render() const;
	virtual void render_ui(uint16_t xpos, uint16_t ypos) const;

	virtual void start_turn();
	virtual bool take_turn(Level *level);
	virtual void end_turn();

	virtual uint8_t get_damage() const;

	void add_action(ActionType at, uint8_t xpos, uint8_t ypos, int8_t value = 0);
	bool actions_empty() const;

	void action_idle();
	bool action_move(Level *level);
	bool action_attack(Level *level);
	bool action_shoot(Level *level);
	bool action_interact();

	void add_ability(const std::string &ability);
	void remove_ability(const std::string &ability);
	bool has_ability(const std::string &ability) const;

	void attack(Actor *other);

	void load_bubble(const std::string &bubble_name, uint8_t timer = 0);
	void clear_bubble();

	StatusType get_status() const { return status; }
	void set_status(StatusType st);

	void set_mount(Mount *m);
	void clear_mount();

	bool get_delete() const { return delete_me; }
	bool get_in_camera() const { return in_camera; }
	bool get_facing_right() const { return facing_right; }

	HoverType get_hovered() const { return hovered; }
	ActorType get_actor_type() const { return actor_type; }
	SDL_Rect get_frame_rect() const { return frame_rect; }

	uint16_t get_ID() const { return actor_ID; }
	uint16_t get_x() const { return x; }
	uint16_t get_y() const { return y; }
	uint8_t get_grid_x() const { return grid_x; }
	uint8_t get_grid_y() const { return grid_y; }
	Mount* get_mount() const { return mount; }
	std::pair<int8_t, int8_t> get_moves() const { return moves; }

	void set_x(uint16_t xpos) { x = xpos; }
	void set_y(uint16_t ypos) { y = ypos; }
	void set_grid_x(uint8_t xpos) { grid_x = xpos; }
	void set_grid_y(uint8_t ypos) { grid_y = ypos; }
	void set_delete(bool del) { delete_me = del; }
	void set_turn_done(bool done) { turn_done = done; }
	void set_hovered(HoverType ht) { hovered = ht; }
	void set_moves(int8_t m) { moves.first = m; }

protected:
	bool delete_me;
	bool in_camera;
	bool turn_done;
	bool facing_right;

	HoverType hovered;
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
	std::pair<int8_t, int8_t> health;
	std::string name;

	SDL_Rect bubble_rect;
	uint8_t bubble_timer;
	Texture *bubble;
	Texture *status_icon;
	StatusType status;

	uint8_t combat_level;
	uint8_t experience;
	std::vector<std::string> abilities;

	SDL_Rect frame_rect;
	Texture *texture;

	Mount *mount;

	double proj_angle;
	uint16_t proj_x, proj_y;
	Texture *projectile;
	std::string proj_name;

	static uint16_t ID;
};

#endif // ACTOR_HPP
