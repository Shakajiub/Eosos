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

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <fstream> // for std::ofstream

enum LogCategory
{
	LOG_NONE,
	LOG_ENGINE,
	LOG_OPTIONS,
	LOG_TEXTURE
};
class Logging
{
public:
	Logging();
	~Logging();

	void init(const std::string &base_path);
	void free();

	void cout(const std::string &text, LogCategory category = LOG_NONE);
	void cerr(const std::string &text, LogCategory category = LOG_NONE);

private:
	bool initialized;
	LogCategory prev_category;

	std::ofstream out;
	std::ofstream err;
};
extern Logging logging;

#endif // LOGGING_HPP
