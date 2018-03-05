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
#include "generator_forest.hpp"
#include "actor_manager.hpp"

GeneratorForest::GeneratorForest()
{

}
GeneratorForest::~GeneratorForest()
{
	free();
}
void GeneratorForest::free()
{

}
void GeneratorForest::init()
{

}
const std::string GeneratorForest::generate(uint8_t depth)
{
	const std::string level =
		"l-0-core/texture/level/floor/dark2_base.png\n"
		"l-1-core/texture/level/floor/dark2_grass.png\n"
		"l-T-core/texture/level/tree/oak_dead.png\n"
		"1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T\n"
		"1T0_0_1_1_1_1_0_0_1_0_1_1_1_0_0_0_0_1_1T\n"
		"1T1_0_0_1_1_0_0_1_0_1_1_0_1_1_0_0_1_1_1T\n"
		"1T1_1_1_1_0_1_0_0_0_1_0_1_1_0_1_1_1_0_1T\n"
		"1T0_1_1_0_0_0_1_0_0_0_1_1_0_1_1_1_0_1_1T\n"
		"1T0_0_1_1_1_1_0_0_1_0_1_1_1_0_0_0_0_1_1T\n"
		"1T1_0_0_1_1_0_0_1_0_1_1_0_1_1_0_0_1_1_1T\n"
		"1T1_1_1_1_0_1_0_0_0_1_0_1_1_0_1_1_1_0_1T\n"
		"1T0_1_1_0_0_0_1_0_0_0_1_1_0_1_1_1_0_1_1T\n"
		"1T1_1_1_1_0_1_0_0_0_1_0_1_1_0_1_1_1_0_1T\n"
		"1T0_0_1_1_1_1_0_0_1_0_1_1_1_0_0_0_0_1_1T\n"
		"1T1_0_0_1_1_0_0_1_0_1_1_0_1_1_0_0_1_1_1T\n"
		"1T1_1_1_1_0_1_0_0_0_1_0_1_1_0_1_1_1_0_1T\n"
		"1T0_1_1_0_0_0_1_0_0_0_1_1_0_1_1_1_0_1_1T\n"
		"1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T1T";
	return level;
}
void GeneratorForest::next_turn(uint16_t turn, ActorManager *am)
{
	std::cout << "forest generator turn #" << (int)turn << std::endl;
}
