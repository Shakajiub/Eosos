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

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

class ActorManager;
class Level;

class Generator
{
public:
	virtual void free() = 0;
	virtual void init() = 0;
	virtual void render_ui() = 0;

	virtual const std::string generate(uint8_t depth) = 0;
	virtual void post_process(ActorManager *am, Level *level, uint8_t depth) = 0;
	virtual void next_turn(ActorManager *am, Level *level) = 0;

	virtual std::pair<uint8_t, uint8_t> get_base_pos() const = 0;
	virtual std::pair<uint8_t, uint8_t> get_spawn_pos() const = 0;
};

#endif // GENERATOR_HPP
