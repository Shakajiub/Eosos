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
#include "ability_shoot.hpp"
#include "hero.hpp"

AbilityShoot::AbilityShoot()
{

}
AbilityShoot::~AbilityShoot()
{
	free();
}
void AbilityShoot::free()
{

}
bool AbilityShoot::init()
{
	if (init_texture("core/texture/ui/icon/bow.png", COLOR_EARTH))
		ability_name = "shoot";
}
void AbilityShoot::apply(Hero *hero)
{

}
