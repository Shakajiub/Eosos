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
#include "particle.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

Particle::Particle() : particle_texture(nullptr)
{

}
Particle::~Particle()
{
	free();
}
void Particle::free()
{
	if (particle_texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(particle_texture->get_name());
		particle_texture = nullptr;
	}
}
void Particle::init(int16_t xpos, int16_t ypos, const std::string &text, uint8_t scale)
{
	// Center the particle on the spawn position
	xpos -= ui.get_bitmap_font()->get_width() * text.length() * scale / 2;
	ypos -= ui.get_bitmap_font()->get_height() * scale / 2;

	x = xpos; y = ypos;
	display_text = text;
	display_scale = scale;
	life_timer = 0;
}
bool Particle::update()
{
	// Don't stay alive for too long
	life_timer += engine.get_dt();
	if (life_timer > 500)
		return false;

	// Just lerp towards the destination position
	const float t = engine.get_dt() / 100.0f * 0.1f;

	x = ((1.0f-t) * x) + (t * dest_x);
	y = ((1.0f-t) * y) + (t * dest_y);

	// Return whether or not the particle is still alive
	if (std::abs(x - dest_x) < 2.0f && std::abs(y - dest_y) < 2.0f)
		return false;
	return true;
}
void Particle::render() const
{
	if (particle_texture != nullptr)
		particle_texture->render(x - camera.get_cam_x(), y - camera.get_cam_y());

	else if (display_text.length() > 0)
	{
		const uint8_t prev_scale = ui.get_bitmap_font()->get_scale();
		ui.get_bitmap_font()->set_color(display_color);
		ui.get_bitmap_font()->set_scale(display_scale);
		ui.get_bitmap_font()->render_text(x - camera.get_cam_x(), y - camera.get_cam_y(), display_text);
		ui.get_bitmap_font()->set_scale(prev_scale);
	}
}
void Particle::set_destination(int16_t xpos, int16_t ypos)
{
	// Center the desired destination
	dest_x = xpos - (ui.get_bitmap_font()->get_width() / 2);
	dest_y = ypos - (ui.get_bitmap_font()->get_height() / 2);
}
