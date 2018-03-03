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

#ifndef PARTICLE_MANAGER_HPP
#define PARTICLE_MANAGER_HPP

#include <vector>

class Particle;

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	void free();

	void update();
	void render() const;

	// TODO - Spawn particles with custom textures
	// TODO - Spawn particles with custom movement parameters

	void spawn_particle(int16_t xpos, int16_t ypos, const std::string &text, SDL_Color color = COLOR_PEPPERMINT, uint8_t scale = 1);

private:
	std::vector<Particle*> particles;
};

#endif // PARTICLE_MANAGER_HPP
