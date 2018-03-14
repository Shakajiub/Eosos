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
#include "logging.hpp"

#include <chrono>
#include <ctime>

Logging logging;

Logging::Logging() : initialized(false), prev_category(LOG_NONE)
{

}
Logging::~Logging()
{
	free();
}
void Logging::init(const std::string &base_path)
{
	// Clear previous logs
	out.open(base_path + "logs/output.txt", std::ofstream::out | std::ofstream::trunc);
	err.open(base_path + "logs/error.txt", std::ofstream::out | std::ofstream::trunc);
	out.close();
	err.close();

	// Set up ofstreams for cout and cerr
	out = std::ofstream(base_path + "logs/output.txt");
	err = std::ofstream(base_path + "logs/error.txt");

	// Print current date and time to both cout and cerr
	std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(p);

	initialized = true;

	cout(std::ctime(&t));
	cerr(std::ctime(&t));
}
void Logging::free()
{
	if (initialized)
	{
		out.close();
		err.close();
	}
	initialized = false;
}
void Logging::cout(const std::string &text, LogCategory category)
{
	if (initialized)
	{
		if (category != prev_category)
		{
			prev_category = category;
			out << std::endl;
		}
		out << text << std::endl;
	}
	else std::cerr << "Logging::cout() Warning! Logging uninitialized!" << std::endl;
}
void Logging::cerr(const std::string &text, LogCategory category)
{
	if (initialized)
	{
		if (category != prev_category)
		{
			prev_category = category;
			out << std::endl;
		}
		err << text << std::endl;
	}
	else std::cerr << "Logging::cerr() Warning! Logging uninitialized!" << std::endl;
}
