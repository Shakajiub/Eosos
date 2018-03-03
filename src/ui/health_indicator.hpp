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

#ifndef HEALTH_INDICATOR_HPP
#define HEALTH_INDICATOR_HPP

class Actor;
class Texture;

class HealthIndicator
{
public:
	HealthIndicator();
	~HealthIndicator();

	void free();
	void init(Actor *new_target, const std::string &heart_texture);

	void update();
	void render() const;

	Actor* get_target() const { return target; }

	void set_target(Actor *new_target) { target = new_target; update(); }
	void set_position(int16_t xpos, int16_t ypos) { x = xpos; y = ypos; }

private:
	int16_t x, y;
	uint8_t shake;
	std::pair<int8_t, int8_t> health;

	Texture *heart_sheet;
	Actor *target;
};

#endif // HEALTH_INDICATOR_HPP