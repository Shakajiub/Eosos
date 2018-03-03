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
#include "particle_manager.hpp"
#include "particle.hpp"

#include <algorithm> // for std::find

ParticleManager::ParticleManager()
{

}
ParticleManager::~ParticleManager()
{
	free();
}
void ParticleManager::free()
{
	for (uint8_t i = 0; i < particles.size(); i++)
	{
		if (particles[i] != nullptr)
			delete particles[i];
	}
	particles.clear();
}
void ParticleManager::update()
{
	Particle *to_erase = nullptr;
	for (Particle *p : particles)
	{
		if (!p->update())
			to_erase = p;
	}
	// This erases just one particle per frame, shouldn't be too bad as long as their amount stays low
	if (to_erase != nullptr)
	{
		std::vector<Particle*>::iterator pos = std::find(particles.begin(), particles.end(), to_erase);
		if (pos != particles.end())
			particles.erase(pos);
		delete to_erase;
	}
}
void ParticleManager::render() const
{
	for (Particle *p : particles)
		p->render();
}
void ParticleManager::spawn_particle(int16_t xpos, int16_t ypos, const std::string &text, SDL_Color color, uint8_t scale)
{
	Particle *new_particle = new Particle;
	new_particle->init(xpos, ypos, text, scale);
	new_particle->set_destination(xpos, ypos - 64);
	new_particle->set_color(color);
	particles.push_back(new_particle);
}
