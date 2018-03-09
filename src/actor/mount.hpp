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

#ifndef MOUNT_HPP
#define MOUNT_HPP

#include "actor.hpp"

class Level;

enum MountClass
{
	MOUNT_SHEEP
};
class Mount : public Actor
{
public:
	Mount();
	~Mount();

	void free();

	virtual void update(Level *level);
	virtual void start_turn();
	virtual bool take_turn(Level *level, ActorManager *am);

	void set_rider(Actor *new_rider);

private:
	Actor *rider;
};

#endif // MOUNT_HPP
