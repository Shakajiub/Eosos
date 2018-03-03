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

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <unordered_map>

class Options
{
public:
	Options();
	~Options();

	void init();
	void load();
	void apply();

	// TODO - Maybe combine these into single templated get/set functions?

	const bool& get_b(const std::string &option);
	const int16_t& get_i(const std::string &option);
	const std::string& get_s(const std::string &option);

	void set_b(const std::string &option, const bool &value);
	void set_i(const std::string &option, const int16_t &value);
	void set_s(const std::string &option, const std::string &value);

private:
	std::unordered_map<std::string, bool> options_b;
	std::unordered_map<std::string, int16_t> options_i;
	std::unordered_map<std::string, std::string> options_s;
};
extern Options options;

#endif // OPTIONS_HPP
