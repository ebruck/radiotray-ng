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

#include <radiotray-ng/playlist/playlist_downloader.hpp>
#include <radiotray-ng/config/config.hpp>
#include <iostream>


int main(int argc, char** argv)
{
	if (argc > 1)
	{
		std::shared_ptr<IConfig> cfg{std::make_shared<Config>("~/.config/radiotray-ng/radiotray-ng.json")};

		cfg->load();

		PlaylistDownloader pld(cfg);

		playlist_t pls;
		IBookmarks::station_data_t std{"", argv[1], "", false, false};
		pld.download_playlist(std, pls);

		for (auto& url: pls)
		{
			std::cout << url << std::endl;
		}
	}

	return 0;
}
