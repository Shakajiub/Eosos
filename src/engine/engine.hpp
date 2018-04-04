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

#ifndef ENGINE_HPP
#define ENGINE_HPP

#ifdef _WIN32
	#define SDL_MAIN_HANDLED
	#include <SDL.h>
	#include <SDL_image.h>
	#include <SDL_mixer.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_image.h>
	#include <SDL2/SDL_mixer.h>
#endif

#include <string>
#include <iostream>
#include <random>

// DawnBringer palette colors
const SDL_Color DAWN_BLACK = { 20, 12, 28 };
const SDL_Color DAWN_PLUM = { 68, 36, 52 };
const SDL_Color DAWN_MIDNIGHT = { 48, 52, 109 };
const SDL_Color DAWN_IRON = { 78, 74, 78 };
const SDL_Color DAWN_EARTH = { 133, 76, 48 };
const SDL_Color DAWN_MOSS = { 52, 101, 36 };
const SDL_Color DAWN_BERRY = { 208, 70, 72 };
const SDL_Color DAWN_OLIVE = { 117, 113, 97 };
const SDL_Color DAWN_CORNFLOWER = { 89, 125, 206 };
const SDL_Color DAWN_OCHER = { 210, 125, 44 };
const SDL_Color DAWN_SLATE = { 133, 149, 161 };
const SDL_Color DAWN_LEAF = { 109, 170, 44 };
const SDL_Color DAWN_PEACH = { 210, 170, 153 };
const SDL_Color DAWN_SKY = { 109, 194, 202 };
const SDL_Color DAWN_MAIZE = { 218, 212, 94 };
const SDL_Color DAWN_PEPPERMINT = { 222, 238, 214 };

class ActorManager;
class SceneManager;
class SoundManager;
class TextureManager;

struct Point
{
	uint8_t x;
	uint8_t y;

	Point(uint8_t xpos, uint8_t ypos)
	{
		x = xpos;
		y = ypos;
	}
};
class Engine
{
public:
	Engine();
	~Engine();

	bool init();
	void close();

	bool update();
	void render() const;

	bool handle_window_event(uint8_t event);
	bool handle_keyboard_input(SDL_Keycode key);

	SDL_Window* get_window() const { return main_window; }
	SDL_Renderer* get_renderer() const { return main_renderer; }

	ActorManager* get_actor_manager() const { return actor_manager; }
	SceneManager* get_scene_manager() const { return scene_manager; }
	SoundManager* get_sound_manager() const { return sound_manager; }
	TextureManager* get_texture_manager() const { return texture_manager; }

	std::string get_base_path() const { return base_path; }
	uint64_t get_rng() { return generator(); }
	uint32_t get_current_time() const { return current_time; }
	uint8_t get_dt() const { return delta_time; }

private:
	SDL_Window *main_window;
	SDL_Renderer *main_renderer;
	SDL_Joystick *main_controller;

	ActorManager *actor_manager;
	SceneManager *scene_manager;
	SoundManager *sound_manager;
	TextureManager *texture_manager;

	std::string base_path;
	std::mt19937 generator;

	uint8_t delta_time;
	uint32_t current_time;
};
extern Engine engine;

#endif // ENGINE_HPP
