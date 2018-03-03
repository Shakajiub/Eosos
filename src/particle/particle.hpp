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

#ifndef PARTICLE_HPP
#define PARTICLE_HPP

class Texture;

class Particle
{
public:
	Particle();
	~Particle();

	void free();
	void init(int16_t xpos, int16_t ypos, const std::string &text, uint8_t scale = 1);

	bool update();
	void render() const;

	void set_destination(int16_t xpos, int16_t ypos);
	void set_color(SDL_Color color) { display_color = color; }

private:
	int16_t x, y;
	int16_t dest_x, dest_y;
	uint16_t life_timer;
	uint8_t display_scale;

	SDL_Color display_color;
	std::string display_text;
	Texture *particle_texture;
};

#endif // PARTICLE_HPP
