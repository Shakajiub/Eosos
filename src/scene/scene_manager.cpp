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
#include "scene_manager.hpp"
#include "scene.hpp"

#include "logging.hpp"
#include "scenario.hpp"
#include "sound_manager.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

SceneManager::SceneManager() : window_focus(true), current_scene(nullptr)
{

}
SceneManager::~SceneManager()
{
	free();
}
void SceneManager::free()
{
	scene_map.clear();
	logging.cout("All scenes erased", LOG_SCENE);
}
void SceneManager::init()
{
	// Must initialize the bitmap font here in case we fail to load a scene
	ui.init_bitmap_font();

	load_scene<Scenario>("test");
	set_scene("test");
}
bool SceneManager::update()
{
	SDL_Event event;
	if (!window_focus)
	{
		if (SDL_WaitEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				window_focus = true;
				return false;
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					case SDL_WINDOWEVENT_RESTORED:
						SDL_SetRelativeMouseMode(SDL_TRUE);
						engine.get_sound_manager()->resume_music();
						window_focus = true;
						break;
					default: break;
				}
			}
		}
		return true;
	}
	if (current_scene == nullptr)
	{
		render();
		while (SDL_WaitEvent(&event))
		{
			if (event.type == SDL_QUIT)
				return false;
			else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
				return false;
			return true;
		}
	}
	return current_scene->update();
}
void SceneManager::render() const
{
	if (!window_focus)
		return;
	if (current_scene == nullptr)
	{
		SDL_RenderClear(engine.get_renderer());

		ui.get_bitmap_font()->render_text(120, 120, "Warning! No scene loaded!");
		ui.get_bitmap_font()->render_text(120, 132, "Press [escape] to quit.");

		SDL_RenderPresent(engine.get_renderer());
	}
	else current_scene->render();
}
template <class T>
bool SceneManager::load_scene(const std::string &scene_name)
{
	auto it = scene_map.find(scene_name);
	if (it == scene_map.end())
	{
		scene_map[scene_name] = std::make_shared<T>();
		scene_map[scene_name]->init();
		return true;
	}
	return false;
}
bool SceneManager::set_scene(const std::string &scene_name)
{
	auto it = scene_map.find(scene_name);
	if (it == scene_map.end())
		return false;

	current_scene = scene_map[scene_name];
	return true;
}
bool SceneManager::free_scene(const std::string &scene_name)
{
	auto it = scene_map.find(scene_name);
	if (it != scene_map.end())
	{
		it->second.reset();
		scene_map.erase(it);
		return true;
	}
	return false;
}
Scenario* SceneManager::get_scene(const std::string &scene_name)
{
	// TODO - Return any scene (not just scenario)

	auto it = scene_map.find(scene_name);
	if (it != scene_map.end())
	{
		Scenario *scene = dynamic_cast<Scenario*>(it->second.get());
		if (scene != NULL)
			return scene;
	}
	return nullptr;
}
