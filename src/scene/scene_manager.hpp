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

#ifndef SCENEMANAGER_HPP
#define SCENEMANAGER_HPP

#include <memory>
#include <unordered_map>

class Scene;
class Scenario;

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	void free();
	void init();

	bool update();
	void render() const;

	template <class T>
	bool load_scene(const std::string &scene_name);
	bool set_scene(const std::string &scene_name);
	bool free_scene(const std::string &scene_name);

	Scenario* get_scene(const std::string &scene_name);
	void set_window_focus(bool focus) { window_focus = focus; }

private:
	bool window_focus;

	std::unordered_map<std::string, std::shared_ptr<Scene> > scene_map;
	std::shared_ptr<Scene> current_scene;
};

#endif // SCENEMANAGER_HPP
