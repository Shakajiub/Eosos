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
#include "actor_manager.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"

#include "camera.hpp"
#include "logging.hpp"
#include "options.hpp"
#include "ui.hpp"

Engine::Engine() :
	main_window(nullptr), main_renderer(nullptr), main_controller(nullptr), delta_time(0), current_time(0),
	actor_manager(nullptr), scene_manager(nullptr), sound_manager(nullptr), texture_manager(nullptr)
{

}
Engine::~Engine()
{

}
bool Engine::init()
{
	//
	//    Initialize core SDL
	//

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
	{
		std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	char *temp_path = SDL_GetBasePath();
	base_path = std::string(temp_path);
	SDL_free(temp_path);

	// Initialize custom logging system && load game options

	logging.init(base_path);
	base_path += "data/";
	options.load();

	//
	//    Initialize main SDL_Window
	//

	uint32_t flags = SDL_WINDOW_SHOWN;
	if (options.get_b("display-fullscreen"))
		flags = flags | SDL_WINDOW_FULLSCREEN_DESKTOP;
	if (options.get_b("display-borderless"))
		flags = flags | SDL_WINDOW_BORDERLESS;

	main_window = SDL_CreateWindow("Eosos", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		options.get_i("display-width"), options.get_i("display-height"), flags
	);
	if (main_window == NULL)
	{
		main_window = nullptr;
		logging.cerr(std::string("Could not create main window! SDL Error: ") + SDL_GetError(), LOG_ENGINE);
		return false;
	}

	//
	//    Initialize main SDL_Renderer
	//

	flags = SDL_RENDERER_ACCELERATED;
	if (options.get_b("display-vsync"))
		flags = flags | SDL_RENDERER_PRESENTVSYNC;

	main_renderer = SDL_CreateRenderer(main_window, -1, flags);
	if (main_renderer == NULL)
	{
		main_renderer = nullptr;
		logging.cerr(std::string("Could not create main renderer! SDL Error: ") + SDL_GetError(), LOG_ENGINE);
		return false;
	}

	// Additional SDL settings

	SDL_SetRenderDrawColor(main_renderer, DAWN_BLACK.r, DAWN_BLACK.g, DAWN_BLACK.b, DAWN_BLACK.a);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	//
	//    Initialize main SDL_Joystick
	//

	if (SDL_NumJoysticks() > 0)
	{
		main_controller = SDL_JoystickOpen(0);
		if (main_controller == NULL)
		{
			logging.cerr(std::string("Could not open game controller! SDL Error: ") + SDL_GetError(), LOG_ENGINE);
			main_controller = nullptr;
		}
	}
	else logging.cout("No joysticks detected ...", LOG_ENGINE);

	//
	//    Initialize SDL_image
	//

	const uint32_t img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags))
	{
		logging.cerr(std::string("SDL_image could not initialize! SDL_image Error: ") + IMG_GetError(), LOG_ENGINE);
		return false;
	}

	//
	//    Initialize SDL_mixer
	//

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		logging.cerr(std::string("SDL_mixer could not initialize! SDL_mixer Error: ") + Mix_GetError(), LOG_ENGINE);
		return false;
	}

	// Initialize other custom engine objects

	generator.seed(std::random_device{}());

	actor_manager = new ActorManager;
	scene_manager = new SceneManager;
	sound_manager = new SoundManager;
	texture_manager = new TextureManager;

	camera.init();
	scene_manager->init();

	return true;
}
void Engine::close()
{
	ui.free();

	if (actor_manager != nullptr)
		delete actor_manager;
	if (sound_manager != nullptr)
		delete sound_manager;
	if (scene_manager != nullptr)
		delete scene_manager;
	if (texture_manager != nullptr)
		delete texture_manager;

	if (main_controller != nullptr)
		SDL_JoystickClose(main_controller);

	SDL_DestroyRenderer(main_renderer);
	SDL_DestroyWindow(main_window);

	Mix_Quit();
	IMG_Quit();
	SDL_Quit();

	logging.free();
}
bool Engine::update()
{
	const uint32_t new_time = SDL_GetTicks();
	delta_time = new_time - current_time;
	current_time = new_time;

	return scene_manager->update();
}
void Engine::render() const
{
	scene_manager->render();

	const int16_t fps_cap = options.get_i("display-fps_cap");
	if (fps_cap > 0) // Apply custom fps cap at the end of the game loop
	{
		const uint8_t frame_delta_time = SDL_GetTicks() - current_time;
		const uint8_t ticks_per_frame = 1000 / fps_cap;

		if (frame_delta_time < ticks_per_frame)
			SDL_Delay(ticks_per_frame - frame_delta_time);
	}
}
bool Engine::handle_window_event(uint8_t event)
{
	bool game_minimized = false;
	switch (event)
	{
		case SDL_WINDOWEVENT_HIDDEN:
		case SDL_WINDOWEVENT_MINIMIZED:
		case SDL_WINDOWEVENT_FOCUS_LOST:
			SDL_SetRelativeMouseMode(SDL_FALSE);
			scene_manager->set_window_focus(false);
			sound_manager->pause_music();
			game_minimized = true;
			break;

		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_RESTORED:
			SDL_SetRelativeMouseMode(SDL_TRUE);
			scene_manager->set_window_focus(true);
			sound_manager->resume_music();
			break;

		default: break;
	}
	return game_minimized;
}
bool Engine::handle_keyboard_input(SDL_Keycode key)
{
	bool input = true;
	switch (key)
	{
		case SDLK_d:
			if (SDL_GetModState() & KMOD_CTRL)
				options.load();
			break;
		case SDLK_c: case SDLK_AUDIONEXT: case SDLK_AUDIOPLAY:
			if (!sound_manager->get_paused())
				sound_manager->skip_song();
			break;
		case SDLK_x: case SDLK_AUDIOMUTE:
			if (sound_manager->get_paused())
				sound_manager->resume_music(true);
			else sound_manager->pause_music(true);
			break;
		default: input = false; break;
	}
	return input;
}
