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
#include "lua_manager.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"
#include "particle_manager.hpp"

#include "camera.hpp"
#include "options.hpp"
#include "ui.hpp"

Engine::Engine() :
	main_window(nullptr), main_renderer(nullptr), delta_time(0), current_time(0), lua_manager(nullptr),
	scene_manager(nullptr), sound_manager(nullptr), texture_manager(nullptr), particle_manager(nullptr)
{

}
Engine::~Engine()
{

}
bool Engine::init()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0)
	{
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	char *temp_path = SDL_GetBasePath();
	base_path = temp_path; base_path += "data/";
	SDL_free(temp_path);

	// Load options from ini file
	options.load();

	// Set up flags for initializing the main SDL_Window
	uint32_t flags = SDL_WINDOW_SHOWN;
	if (options.get_b("display-fullscreen"))
		flags = flags | SDL_WINDOW_FULLSCREEN_DESKTOP;
	if (options.get_b("display-borderless"))
		flags = flags | SDL_WINDOW_BORDERLESS;

	// Create main window
	main_window = SDL_CreateWindow("Eosos", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		options.get_i("display-width"), options.get_i("display-height"), flags
	);
	if (main_window == NULL)
	{
		main_window = nullptr;
		std::cout << "window could not be created! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	// Set up flags for initializing main SDL_Renderer
	flags = SDL_RENDERER_ACCELERATED;
	if (options.get_b("display-vsync"))
		flags = flags | SDL_RENDERER_PRESENTVSYNC;

	// Create main renderer
	main_renderer = SDL_CreateRenderer(main_window, -1, flags);
	if (main_renderer == NULL)
	{
		main_renderer = nullptr;
		std::cout << "renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	// Additional SDL settings
	SDL_SetRenderDrawColor(main_renderer, 20, 12, 28, 255);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	// Initialize SDL_image
	const uint32_t img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags))
	{
		std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
		return false;
	}
	// Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
		return false;
	}
	//Mix_ReserveChannels(4);

	// Initialize randomization features
	generator.seed(std::mt19937::default_seed);

	// Create the main engine managers
	lua_manager = new LuaManager;
	scene_manager = new SceneManager;
	sound_manager = new SoundManager;
	texture_manager = new TextureManager;
	particle_manager = new ParticleManager;

	camera.init();

	// Initialize bitmap_font
	if (!ui.init_bitmap_font())
		return false;

	// Load the main game_scene
	scene_manager->init();

	return true;
}
void Engine::close()
{
	ui.free();

	if (sound_manager != nullptr)
		delete sound_manager;
	if (particle_manager != nullptr)
		delete particle_manager;
	if (scene_manager != nullptr)
		delete scene_manager;
	if (texture_manager != nullptr)
		delete texture_manager;
	if (lua_manager != nullptr)
		delete lua_manager;

	SDL_DestroyRenderer(main_renderer);
	SDL_DestroyWindow(main_window);

	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
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

	const uint8_t frame_delta_time = SDL_GetTicks() - current_time;
	const uint8_t ticks_per_frame = 1000 / options.get_i("display-fps_cap");

	if (frame_delta_time < ticks_per_frame) // Apply fps cap at the end of the entire game loop
		SDL_Delay(ticks_per_frame - frame_delta_time);
}
