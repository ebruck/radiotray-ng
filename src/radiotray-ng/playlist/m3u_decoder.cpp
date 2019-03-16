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

#include <radiotray-ng/playlist/m3u_decoder.hpp>
#include <sstream>


std::string M3uDecoder::get_name()
{
	return "M3U";
}


bool M3uDecoder::is_decodable(const std::string& content_type, const std::string& /*content*/)
{
	return (content_type.find("audio/mpegurl"  ) != std::string::npos ||
			content_type.find("audio/x-mpegurl") != std::string::npos ||
			content_type.find("text/html")       != std::string::npos ||
			content_type.find("text/uri-list")   != std::string::npos ||
			content_type.find("application/vnd.apple.mpegurl") != std::string::npos);
}


bool M3uDecoder::decode(const std::string& content_type, const std::string& content, playlist_t& playlist)
{
	playlist.clear();

	if (content.empty())
	{
		return false;
	}

	bool uri_list = (content_type.find("text/html") != std::string::npos ||
		content_type.find("text/uri-list") != std::string::npos);

	std::string tmp(content);
	std::replace(tmp.begin(), tmp.end(), '\r', '\n');
	std::stringstream ss(tmp);

	std::string line;
	while(std::getline(ss, line, '\n'))
	{
		if (!line.empty())
		{
			if (line[0] != '#')
			{
				if (uri_list)
				{
					// if text/html or text/uri-list...
					if (line.find("http://") != 0)
					{
						continue;
					}
				}

				playlist.push_back(radiotray_ng::trim(line));
			}
		}
	}

	return !playlist.empty();
}
