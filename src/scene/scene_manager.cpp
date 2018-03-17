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
#include "menu.hpp"
#include "scenario.hpp"

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
	load_scene<Menu>("menu");
	load_scene<Scenario>("scenario");

	set_scene("menu");
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
				engine.handle_window_event(event.window.event);
		}
		return true;
	}
	if (current_scene == nullptr)
	{
		logging.cerr("No scene loaded! Exiting ...", LOG_SCENE);
		return false;
	}
	else return current_scene->update();
}
void SceneManager::render() const
{
	if (window_focus && current_scene != nullptr)
		current_scene->render();
}
template <class T>
bool SceneManager::load_scene(const std::string &scene_name)
{
	auto it = scene_map.find(scene_name);
	if (it == scene_map.end())
	{
		scene_map[scene_name] = std::make_shared<T>();
		return true;
	}
	return false;
}
bool SceneManager::set_scene(const std::string &scene_name)
{
	auto it = scene_map.find(scene_name);
	if (it == scene_map.end())
		return false;

	if (current_scene != nullptr)
		current_scene->free();

	current_scene = scene_map[scene_name];

	if (current_scene != nullptr)
		current_scene->init();

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
	auto it = scene_map.find(scene_name);
	if (it != scene_map.end())
	{
		Scenario *scene = dynamic_cast<Scenario*>(it->second.get());
		if (scene != NULL)
			return scene;
	}
	return nullptr;
}
