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

class AStar;

enum MonsterClass
{
	MONSTER_NONE,
	MONSTER_MISC_LIVING_OAK,
	MONSTER_MISC_LIVING_SPRUCE,
	MONSTER_DWARF_WARRIOR,
	MONSTER_DWARF_NECROMANCER,
	MONSTER_DWARF_BEASTMASTER,
	MONSTER_DWARF_KING,
	MONSTER_KOBOLD_WARRIOR,
	MONSTER_KOBOLD_ARCHER,
	MONSTER_KOBOLD_MAGE,
	MONSTER_KOBOLD_DEMONIAC,
	MONSTER_KOBOLD_TRUEFORM,
	MONSTER_UNDEAD_ZOMBIE,
	MONSTER_UNDEAD_VAMPIRE,
	MONSTER_UNDEAD_GHOST,
	MONSTER_UNDEAD_MUMMY
};
class Monster : public Actor
{
public:
	Monster();
	~Monster();

	void free();
	virtual bool init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name);

	virtual void render() const;
	virtual void start_turn();
	virtual bool take_turn(Level *level);
	virtual void end_turn();

	bool init_class(MonsterClass mc);
	bool init_healthbar();
	bool init_pathfinder();
	void step_pathfinder(Level *level);

private:
	AStar *pathfinder;
	Texture *healthbar;

	MonsterClass monster_class;
};

#endif // MONSTER_HPP
