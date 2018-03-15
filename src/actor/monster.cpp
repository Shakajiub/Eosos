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
#include "monster.hpp"
#include "actor_manager.hpp"
#include "level.hpp"
#include "astar.hpp"

#include "mount.hpp"
#include "camera.hpp"
#include "sound_manager.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"
#include "message_log.hpp"
#include "ui.hpp"

uint16_t distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	return (uint16_t)SDL_sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}
Monster::Monster() : pathfinder(nullptr), healthbar(nullptr), monster_class(MONSTER_NONE), spell_timer(0)
{
	health = std::make_pair(3, 3);
	name = "???";
}
Monster::~Monster()
{
	free();
}
void Monster::free()
{
	if (pathfinder != nullptr)
	{
		delete pathfinder;
		pathfinder = nullptr;
	}
	if (healthbar != nullptr)
	{
		engine.get_texture_manager()->free_texture(healthbar->get_name());
		healthbar = nullptr;
	}
}
bool Monster::init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name)
{
	if (!Actor::init(at, xpos, ypos, texture_name))
		return false;

	return (init_pathfinder() && init_healthbar());
}
void Monster::render() const
{
	Actor::render();

	if (healthbar != nullptr && in_camera && health.first > 0 &&
		((hovered != HOVER_NONE) || health.first < health.second))
	{
		const uint8_t hp_percent = (float)health.first / (float)health.second * 14;
		const SDL_Rect temp_rect = { 28 - (hp_percent * 2), 0, 3, 16 };

		healthbar->render(
			x - camera.get_cam_x() + (facing_right ? 0 : 26),
			y - camera.get_cam_y(),
			&temp_rect, 2, SDL_FLIP_NONE, 0.0
		);
	}
}
void Monster::death(ActorManager *am, Level *level)
{
	if (monster_class == MONSTER_KOBOLD_DEMONIAC)
	{
		Actor * temp = am->spawn_actor(level, ACTOR_MONSTER, grid_x, grid_y);
		dynamic_cast<Monster*>(temp)->init_class(MONSTER_KOBOLD_TRUEFORM);
		level->set_turn(0);

		ui.spawn_message_box("BOSS", "Kobold Trueform");
		if (ui.get_message_log() != nullptr)
			ui.get_message_log()->add_message("The Kobold Demoniac transforms!", DAWN_MAIZE);
		engine.get_sound_manager()->set_playlist(PT_BOSS);
	}
}
void Monster::start_turn()
{
	turn_done = false;

	const uint8_t temp_moves = (mount != nullptr) ? max_moves + 1 : max_moves;
	moves = std::make_pair(temp_moves, temp_moves);

	if (pathfinder != nullptr)
		pathfinder->clear_path();

	if (spell_timer > 0)
		spell_timer -= 1;
}
bool Monster::take_turn(Level *level, ActorManager *am)
{
	if (turn_done)
	{
		if (level->get_wall_type(current_action.xpos, current_action.ypos) == NT_BASE)
		{
			if (monster_class == MONSTER_PEST_SCORPION || monster_class == MONSTER_KOBOLD_TRUEFORM ||
				monster_class == MONSTER_DWARF_KING || monster_class == MONSTER_PLATINO)
				level->set_damage_base(20);
			else level->set_damage_base(1);

			turn_done = true;
			delete_me = true;
			moves.first = 0;
			return true;
		}
		if (moves.first > 0)
			turn_done = false;
	}
	if (action_queue.empty())
	{
		if (moves.first > 0 && has_ability("shoot"))
		{
			const int8_t offset_x[12] = { -1, 0, 1, -2, -2, -2, 2, 2, 2, -1, 0, 1 };
			const int8_t offset_y[12] = { -2, -2, -2, -1, 0, 1, -1, 0, 1, 2, 2, 2 };

			for (uint8_t i = 0; i < 12; i++)
			{
				uint8_t block_x = grid_x + offset_x[i];
				if (offset_x[i] == -2) block_x += 1;
				else if (offset_x[i] == 2) block_x -= 1;

				uint8_t block_y = grid_y + offset_y[i];
				if (offset_y[i] == -2) block_y += 1;
				else if (offset_y[i] == 2) block_y -= 1;

				if (!level->get_wall(block_x, block_y))
				{
					Actor *temp_actor = level->get_actor(grid_x + offset_x[i], grid_y + offset_y[i]);
					if (temp_actor != nullptr && temp_actor->get_actor_type() == ACTOR_HERO)
					{
						add_action(ACTION_SHOOT, grid_x + offset_x[i], grid_y + offset_y[i]);
						moves.first = 0;
						break;
					}
				}
			}
		}
		if (moves.first > 0 && has_ability("weakness") && spell_timer == 0)
		{
			std::vector<Actor*> targets;
			for (int8_t ypos = -2; ypos < 3; ypos++)
			{
				for (int8_t xpos = -2; xpos < 3; xpos++)
				{
					Actor *temp_actor = level->get_actor(grid_x + xpos, grid_y + ypos);
					if (temp_actor != nullptr && temp_actor->get_actor_type() == ACTOR_HERO)
					{
						if (temp_actor->get_status() != STATUS_WEAK)
							targets.push_back(temp_actor);
					}
				}
			}
			if (targets.size() > 0)
			{
				Actor *temp_actor = targets[targets.size() - 1];
				temp_actor->set_status(STATUS_WEAK);

				add_action(ACTION_INTERACT, grid_x, grid_y);
				spell_timer = 5;
				moves.first = 0;

				if (ui.get_message_log() != nullptr)
					ui.get_message_log()->add_message("The " + name + " casts %6Weakness%9!", DAWN_OCHER);
			}
		}
		if (moves.first > 0 && has_ability("necromancy") && spell_timer == 0)
		{
			if (am != nullptr)
			{
				Actor *spawn = am->spawn_actor(level, ACTOR_MONSTER, grid_x, grid_y);
				if (spawn != nullptr)
				{
					if (engine.get_rng() % 10 != 0)
						dynamic_cast<Monster*>(spawn)->init_class(MONSTER_SKELETON);
					else dynamic_cast<Monster*>(spawn)->init_class(MONSTER_SKELETON_DISEASED);

					add_action(ACTION_INTERACT, grid_x, grid_y);
					spell_timer = 4;
					moves.first = 0;

					if (ui.get_message_log() != nullptr)
						ui.get_message_log()->add_message("The " + name + " summons a minion!", DAWN_OCHER);
				}
			}
		}
		if (moves.first > 0 && pathfinder != nullptr)
		{
			if (!pathfinder->get_path_found())
			{
				auto base_pos = level->get_base_pos();
				pathfinder->find_path(level, Point(grid_x, grid_y), Point(base_pos.first, base_pos.second), ACTOR_MONSTER);
			}
			step_pathfinder(level);
		}
	}
	return Actor::take_turn(level, am);
}
void Monster::end_turn()
{
	Actor::end_turn();
}
bool Monster::init_class(MonsterClass mc)
{
	std::string class_texture = "actor/missing.png";
	monster_class = mc;

	// Ideally this stuff should be loaded from an external monster definion file,
	// but I have no time. Will be re-done for the post-7DRL cleaned-up version.
	// For now, enjoy this disgusting switch case.

	switch (monster_class)
	{
		case MONSTER_PLATINO:
			name = "Platino";
			class_texture = "actor/dragon_de_platino.png";
			health = std::make_pair(20, 20);
			proj_type = PROJECTILE_WITHER;
			max_damage = 3;
			break;
		case MONSTER_PEST_ANT:
			name = "Giant Ant";
			class_texture = "actor/pest_ant.png";
			break;
		case MONSTER_PEST_BUG:
			name = "Big Bug";
			class_texture = "actor/pest_bug.png";
			set_status(STATUS_ARMORED);
			break;
		case MONSTER_PEST_BEE:
			name = "Massive Bee";
			class_texture = "actor/pest_bee.png";
			max_moves = 2;
			break;
		case MONSTER_PEST_SCORPION:
			name = "Humongous Scorpion";
			class_texture = "actor/pest_scorpion.png";
			proj_type = PROJECTILE_DART;
			health = std::make_pair(8, 8);
			break;
		case MONSTER_KOBOLD_WARRIOR:
			name = "Kobold Warrior";
			class_texture = "actor/kobold_warrior.png";
			break;
		case MONSTER_KOBOLD_ARCHER:
			name = "Kobold Archer";
			class_texture = "actor/kobold_archer.png";
			proj_type = PROJECTILE_ARROW;
			add_ability("shoot");
			break;
		case MONSTER_KOBOLD_MAGE:
			name = "Kobold Mage";
			class_texture = "actor/kobold_mage.png";
			add_ability("weakness");
			spell_timer = 2;
			break;
		case MONSTER_KOBOLD_DEMONIAC:
			name = "Kobold Demoniac";
			class_texture = "actor/kobold_demoniac.png";
			proj_type = PROJECTILE_WITHER;
			break;
		case MONSTER_KOBOLD_TRUEFORM:
			name = "Kobold Trueform";
			class_texture = "actor/kobold_trueform.png";
			proj_type = PROJECTILE_FIREBALL;
			health = std::make_pair(8, 8);
			add_ability("shoot");
			max_moves = 2;
			break;
		case MONSTER_DWARF_WARRIOR:
			name = "Dwarven Warrior";
			class_texture = "actor/dwarf_warrior.png";
			set_status(STATUS_ARMORED);
			break;
		case MONSTER_DWARF_NECROMANCER:
			name = "Dwarven Necromancer";
			class_texture = "actor/dwarf_necromancer.png";
			add_ability("necromancy");
			spell_timer = 2;
			break;
		case MONSTER_DWARF_BEASTMASTER:
			name = "Dwarven Beastmaster";
			class_texture = "actor/dwarf_beastmaster.png";
			proj_type = PROJECTILE_DART;
			add_ability("shoot");
			break;
		case MONSTER_DWARF_KING:
			name = "Dwarven King";
			class_texture = "actor/dwarf_king.png";
			health = std::make_pair(12, 12);
			max_damage = 2;
			break;
		case MONSTER_DEMON_RED:
			name = "Demon";
			class_texture = "actor/demon_red.png";
			health = std::make_pair(4, 4);
			set_status(STATUS_REGEN);
			max_damage = 2;
			break;
		case MONSTER_DEMON_HORNED:
			name = "Horned Demon";
			class_texture = "actor/demon_horned.png";
			health = std::make_pair(4, 4);
			max_damage = 2;
			break;
		case MONSTER_DEMON_PLATINUM:
			name = "Platinum Demon";
			class_texture = "actor/demon_platinum.png";
			health = std::make_pair(5, 5);
			proj_type = PROJECTILE_WITHER;
			max_damage = 2;
			break;
		case MONSTER_DEMON_FLYING:
			name = "Flying Demon";
			class_texture = "actor/demon_flying.png";
			health = std::make_pair(4, 4);
			max_damage = 2;
			max_moves = 2;
			break;
		case MONSTER_DEMON_FIRE:
			name = "Fire Demon";
			class_texture = "actor/demon_fire.png";
			health = std::make_pair(4, 4);
			proj_type = PROJECTILE_FIREBALL;
			add_ability("shoot");
			max_damage = 2;
			break;
		case MONSTER_SKELETON:
			name = "Skeleton";
			class_texture = "actor/skeleton.png";
			break;
		case MONSTER_SKELETON_DISEASED:
			name = "Diseased Skeleton";
			class_texture = "actor/skeleton_diseased.png";
			proj_type = PROJECTILE_DART;
			break;
		default: monster_class = MONSTER_NONE; break;
	}
	if (texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(texture->get_name());
		texture = nullptr;
	}
	texture = engine.get_texture_manager()->load_texture(class_texture);
	return texture != nullptr;
}
bool Monster::init_healthbar()
{
	healthbar = engine.get_texture_manager()->load_texture("ui/health_bar.png");
	return healthbar != nullptr;
}
bool Monster::init_pathfinder()
{
	pathfinder = new AStar;
	return pathfinder->init();
}
void Monster::step_pathfinder(Level *level)
{
	Actor *temp_actor = level->get_actor(pathfinder->get_goto_x(), pathfinder->get_goto_y());
	if (temp_actor != nullptr)
	{
		if (temp_actor->get_actor_type() == ACTOR_HERO || temp_actor->get_actor_type() == ACTOR_PROP)
			add_action(ACTION_ATTACK, pathfinder->get_goto_x(), pathfinder->get_goto_y());

		else if (temp_actor->get_actor_type() == ACTOR_MOUNT)
		{
			if (mount == nullptr)
			{
				set_mount(dynamic_cast<Mount*>(temp_actor));
				add_action(ACTION_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());
				pathfinder->step();
			}
			else turn_done = true;
		}
		else turn_done = true;
		moves.first = 0;
	}
	else
	{
		moves.first -= 1;
		add_action(ACTION_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());
		pathfinder->step();
	}
}
