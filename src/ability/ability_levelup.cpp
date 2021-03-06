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
#include "ability_levelup.hpp"
#include "hero.hpp"

#include "level_up_box.hpp"
#include "widget.hpp"
#include "ui.hpp"

AbilityLevelUp::AbilityLevelUp()
{
	ability_desc = "Level up!";
}
AbilityLevelUp::~AbilityLevelUp()
{
	free();
}
void AbilityLevelUp::free()
{

}
bool AbilityLevelUp::init()
{
	if (init_texture("ui/icon/arrow_up.png", DAWN_SKY))
		ability_name = "level-up";

	return true;
}
void AbilityLevelUp::apply(Hero *hero, bool cancel)
{
	if (hero != nullptr && !cancel)
	{
		Widget *box = ui.spawn_widget<LevelUpBox>("level-up-box");
		if (box != nullptr)
			dynamic_cast<LevelUpBox*>(box)->init(hero);
	}
}
