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

#ifndef GENERATOR_FOREST_HPP
#define GENERATOR_FOREST_HPP

#include "generator.hpp"

class GeneratorForest : public Generator
{
public:
	GeneratorForest();
	~GeneratorForest();

	virtual void free();
	virtual void init();

	virtual const std::string generate(uint8_t depth);
	virtual void next_turn(uint16_t turn, ActorManager *am);
};

#endif // GENERATOR_FOREST_HPP
