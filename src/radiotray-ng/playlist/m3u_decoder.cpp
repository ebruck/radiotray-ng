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


std::string M3uDecoder::get_name()
{
	return "M3U";
}


bool M3uDecoder::is_decodable(const std::string& content_type, const std::string& /*content*/)
{
	return (content_type.find("audio/mpegurl"  ) != std::string::npos ||
	        content_type.find("audio/x-mpegurl") != std::string::npos);
}


bool M3uDecoder::decode(const std::string& content, playlist_t& playlist)
{
	if (content.empty())
	{
		return false;
	}

	playlist.clear();

	std::stringstream ss(content);
	std::string line;

	char line_terminator = radiotray_ng::guess_line_terminator(content);

	while(std::getline(ss, line, line_terminator))
	{
		if (!line.empty())
		{
			if (line[0] != '#')
			{
				playlist.push_back(radiotray_ng::trim(line));
			}
		}
	}

	return !playlist.empty();
}
