// Copyright 2017 Edward G. Bruck <ed.bruck1@gmail.com>
//
// This file is part of Radiotray-NG.
//
// Radiotray-NG is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Radiotray-NG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Radiotray-NG.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <locale>
#include <string>
#include <wordexp.h>
#include <basedir.h>

#include <fstream>
#include <boost/filesystem.hpp>


namespace radiotray_ng
{
	inline std::string& ltrim(std::string& str)
	{
		auto it2 = std::find_if(str.begin(), str.end(), [](char ch){ return !std::isspace<char>(ch , std::locale::classic()); });
		str.erase(str.begin(), it2);
		return str;
	}


	inline std::string& rtrim(std::string& str)
	{
		auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch){ return !std::isspace<char>(ch , std::locale::classic()); });
		str.erase(it1.base(), str.end());
		return str;
	}


	inline std::string& trim(std::string& str)
	{
		return ltrim(rtrim(str));
	}


	inline std::string word_expand(const std::string& s)
	{
		std::string tmp;

		if (!s.empty())
		{
			wordexp_t exp_result;
			wordexp(s.c_str(), &exp_result, 0);
			tmp = exp_result.we_wordv[0];
			wordfree(&exp_result);
		}
		return tmp;
	}


	inline char guess_line_terminator(const std::string& str)
	{
		return str.find('\n') != std::string::npos ? '\n' : '\r';
	}


	inline std::string word_wrap(std::string str, size_t width)
	{
	    size_t curWidth = width;

	    while (curWidth < str.length())
	    {
	        std::string::size_type spacePos = str.rfind(' ', curWidth);

	        if (spacePos == std::string::npos)
	        {
	            spacePos = str.find(' ', curWidth);

	            if (spacePos == std::string::npos)
	            {
	            	break;
	            }
	        }

	        if (spacePos != std::string::npos)
	        {
	            str[spacePos] = '\n';
	            curWidth = spacePos + width + 1;
	        }
	    }

	    return str;
	}


	inline std::string path_to_filename(const std::string& path)
	{
		return path.substr(path.rfind("/")+1);
	}


	inline std::string to_upper_copy(std::string s)
	{
	    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	    return s;
	}


	inline void load_string_file(const std::string& p, std::string& str)
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		file.open(p, std::ios_base::binary);
		std::size_t sz = static_cast<std::size_t>(boost::filesystem::file_size(p));
		str.resize(sz, '\0');
		file.read(&str[0], sz);
	}


	inline std::string get_data_dir(const std::string& app_name)
	{
		std::string xdg_data_home_dir = xdgConfigHome(nullptr);
		xdg_data_home_dir += std::string("/") + app_name + std::string("/");
		return xdg_data_home_dir;
	}


	inline std::string to_lower(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}
}
