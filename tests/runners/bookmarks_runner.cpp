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

#include <radiotray-ng/bookmarks/bookmarks.hpp>
#include <json/json.h>
#include <iostream>
#include <fstream>


void print_group(const IBookmarks::group_data_t& group_data)
{
	std::cout << group_data.group << std::endl;
	std::cout << group_data.image << std::endl;

	for(const IBookmarks::station_data_t& s : group_data.stations)
	{
		std::cout << "   └── " << s.name  << std::endl;
		std::cout << "       " << s.url   << std::endl;
		std::cout << "       " << s.image << std::endl;
	}
}


int main(int argc, char** argv)
{
	if (argc > 1)
	{
		Bookmarks bm(argv[1]);
		bm.load();

		for(size_t i = 0; i < bm.size(); ++i)
		{
			print_group(bm[i]);
		}

		// save?
		if (argc == 3)
		{
			bm.save();
		}
	}

	return 0;
}
