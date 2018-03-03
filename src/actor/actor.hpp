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
#include <unordered_map>

class Ability;
class Mount;
class Dungeon;
class Level;
class Texture;

enum Skill
{
	A_STR, A_DEX, A_CON,
	A_INT, A_WIS, A_CHA,
	A_LCK
};
enum ActionType
{
	AT_MOVE,
	AT_ATTACK,
	AT_INTERACT
};
enum ActorType
{
	ACTOR_NULL,
	ACTOR_HERO,
	ACTOR_MONSTER,
	ACTOR_MOUNT
};
typedef struct
{
	ActionType action_type;
	uint8_t xpos, ypos;
}
Action;

class Actor
{
public:
	Actor();
	~Actor();

	void free();
	bool init(const std::string &actor_name, uint8_t xpos, uint8_t ypos, Texture *sheet);
	bool init(const std::string &actor_name, uint8_t xpos, uint8_t ypos, const std::string &texture_name);

	static Actor* find(Dungeon *scene, uint8_t xpos, uint8_t ypos);

	virtual void update(Dungeon *scene);
	virtual void render() const;
	virtual void start_turn();
	virtual bool take_turn(Dungeon *scene);
	virtual void end_turn();

	void add_ability(const std::string &name, Ability *ability);
	bool use_ability(const std::string &name, uint8_t value);

	void add_action(ActionType at, uint8_t xpos, uint8_t ypos);
	void action_idle();
	bool action_move(Dungeon *scene);
	bool action_attack(Dungeon *scene);
	bool action_interact(Dungeon *scene);

	void attack(Dungeon *scene, Actor *other);
	virtual std::pair<uint8_t, bool> get_melee_damage() const;
	virtual uint8_t get_armor_value() const;

	void take_damage(Level *level, int8_t damage, const std::string &source);

	void load_bubble(const std::string &bubble_name, uint8_t timer = 0);
	void render_bubble() const;

	void turn_around() { facing_right = !facing_right; }

	void init_mount();
	void toggle_mount();
	void grant_movement(uint8_t movement) { moves.first += movement; }

	bool get_behind(const Actor *other) const;
	bool get_overlap(uint8_t xpos, uint8_t ypos) const { return (xpos == grid_x && ypos == grid_y); }
	std::pair<int8_t, int8_t> get_health() const { return health; }
	std::pair<int8_t, int8_t> get_moves() const { return moves; }
	auto& get_abilities() const { return abilities; }
	Action* get_action() const { return current_action; }

	bool get_mount_available() const { return has_mount; }
	Mount* get_mount() const { return mount; }

	bool get_player_visible(Dungeon *scene) const;
	bool get_in_camera() const { return in_camera; }
	bool get_facing_right() const { return facing_right; }
	uint16_t get_x() const { return x; }
	uint16_t get_y() const { return y; }
	uint16_t get_ID() const { return actor_ID; }
	uint8_t get_grid_x() const { return grid_x; }
	uint8_t get_grid_y() const { return grid_y; }
	uint8_t get_direction() const { return move_direction; }
	uint8_t get_visibility_range() const { return visibility_range; }
	SDL_Rect get_frame_rect() const { return frame_rect; }

	void set_x(uint16_t new_x) { x = new_x; }
	void set_y(uint16_t new_y) { y = new_y; }
	void set_position(uint8_t new_x, uint8_t new_y, Dungeon *scene = nullptr);
	void set_direction(uint8_t direction) { move_direction = direction; }
	void set_visibility_range(uint8_t range) { visibility_range = range; }
	void set_texture(Texture *new_texture) { texture = new_texture; }
	void set_skill(const uint8_t skill, const int8_t value);
	void set_skills(const int8_t scores[7])
	{
		for (uint8_t i = 0; i < 7; i++)
			set_skill(i, scores[i]);
	}
	void set_health(std::pair<int8_t, int8_t> new_health) { health = new_health; }

protected:
	bool in_camera;
	bool turn_done;
	bool facing_right;
	bool has_mount;

	double render_angle;

	uint16_t x, y;
	uint16_t actor_ID;
	uint8_t grid_x, grid_y;
	uint8_t prev_x, prev_y;
	uint8_t move_direction;
	uint8_t bubble_timer;
	uint8_t anim_frames;
	uint8_t anim_timer;

	std::string name;
	int8_t skills[7];
	std::pair<int8_t, int8_t> health;
	std::pair<int8_t, int8_t> moves;
	std::unordered_map<std::string, Ability*> abilities;

	uint8_t visibility_range;

	Action *current_action;
	std::queue<Action*> action_queue;

	SDL_Rect frame_rect;
	Texture *texture;
	Texture *bubble;
	Mount *mount;

	static uint16_t ID;
};

#endif // ACTOR_HPP
