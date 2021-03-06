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

#ifndef MONSTER_HPP
#define MONSTER_HPP

#include "actor.hpp"

//class AStar;

enum MonsterClass
{
	MONSTER_NONE,
	MONSTER_PLATINO,
	MONSTER_PEST_ANT,
	MONSTER_PEST_BUG,
	MONSTER_PEST_BEE,
	MONSTER_PEST_SCORPION,
	MONSTER_KOBOLD_WARRIOR,
	MONSTER_KOBOLD_ARCHER,
	MONSTER_KOBOLD_MAGE,
	MONSTER_KOBOLD_DEMONIAC,
	MONSTER_KOBOLD_TRUEFORM,
	MONSTER_DWARF_WARRIOR,
	MONSTER_DWARF_NECROMANCER,
	MONSTER_DWARF_BEASTMASTER,
	MONSTER_DWARF_KING,
	MONSTER_DEMON_RED,
	MONSTER_DEMON_HORNED,
	MONSTER_DEMON_PLATINUM,
	MONSTER_DEMON_FLYING,
	MONSTER_DEMON_FIRE,
	MONSTER_SKELETON,
	MONSTER_SKELETON_DISEASED
};
class Monster : public Actor
{
public:
	Monster();
	~Monster();

	void free();

	virtual bool init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name);
	virtual void render() const;
	virtual void death(Level *level);

	virtual void start_turn();
	virtual bool take_turn(Level *level);
	virtual void end_turn();

	virtual void interact(Level *level, Point pos);

	bool init_class(MonsterClass mc);
	bool init_healthbar();
	//bool init_pathfinder();
	//void step_pathfinder(Level *level);

	MonsterClass get_monster_class() const { return monster_class; }

private:
	//AStar *pathfinder;
	Texture *healthbar;

	MonsterClass monster_class;
	uint8_t spell_timer;
};

#endif // MONSTER_HPP
