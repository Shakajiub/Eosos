//	Copyright (C) 2018 Jere Oikarinen
//
//	This file is part of DawnRPG.
//
//	DawnRPG is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	DawnRPG is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with DawnRPG. If not, see <http://www.gnu.org/licenses/>.

#ifndef PROP_HPP
#define PROP_HPP

#include "actor.hpp"

class Prop : public Actor
{
public:
	Prop();
	~Prop();

	virtual bool take_turn(Level *level, ActorManager *am);
};

#endif // PROP_HPP
