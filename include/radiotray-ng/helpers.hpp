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
		if (!s.empty())
		{
			// any spaces then we can't use wordexp...
			if (s.find(' ') == std::string::npos)
			{
				std::string tmp;
				wordexp_t exp_result;
				wordexp(s.c_str(), &exp_result, 0);
				tmp = exp_result.we_wordv[0];
				wordfree(&exp_result);
				return tmp;
			}
		}
		return s;
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

	        str[spacePos] = '\n';
	        curWidth = spacePos + width + 1;
	    }

	    return str;
	}


	inline std::string basename(const std::string& path)
	{
		return path.substr(path.rfind("/")+1);
	}


	inline std::string get_data_dir(const std::string& app_name)
	{
		std::string xdg_data_home_dir = xdgConfigHome(nullptr);
		xdg_data_home_dir += std::string("/") + app_name + std::string("/");
		return xdg_data_home_dir;
	}


	inline std::string get_cache_dir(const std::string& app_name)
	{
		std::string xdg_cache_home_dir = xdgCacheHome(nullptr);
		xdg_cache_home_dir += std::string("/") + app_name + std::string("/");
		return xdg_cache_home_dir;
	}


	inline std::string get_runtime_dir()
	{
		xdgHandle xdg_handle;
		std::string runtime_dir;

		if (xdgInitHandle(&xdg_handle))
		{
			// https://bugs.launchpad.net/ubuntu/+source/libxdg-basedir/+bug/1821670
			// Not sure why non-cached call uses XDG_RUNTIME_DIRECTORY environment
			// variable instead of XDG_RUNTIME_DIR...
			auto xdg_runtime_dir = xdgRuntimeDirectory(&xdg_handle);
			if (xdg_runtime_dir)
			{
				runtime_dir = xdg_runtime_dir;
			}
			xdgWipeHandle(&xdg_handle);
		}

		return runtime_dir;
	}


	inline std::string to_lower(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}
}
