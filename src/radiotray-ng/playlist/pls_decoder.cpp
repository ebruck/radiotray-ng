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

#include <radiotray-ng/playlist/pls_decoder.hpp>


std::string PlsDecoder::get_name()
{
	return "PLS";
}


bool PlsDecoder::is_decodable(const std::string& content_type, const std::string& content)
{
	return (content_type.find("audio/x-scpls"      ) != std::string::npos ||
	        content_type.find("application/pls+xml") != std::string::npos ||
	        content.find("[playlist]")               != std::string::npos);
}


bool PlsDecoder::decode(const std::string& content, playlist_t& playlist)
{
	if (content.empty())
	{
		return false;
	}

	playlist.clear();

	std::stringstream ss(content);
	const std::string file_entry = "File";

	std::string line;
	char line_terminator = radiotray_ng::guess_line_terminator(content);

	while(std::getline(ss, line, line_terminator))
	{
		if (line.compare(0, file_entry.length(), file_entry) == 0)
		{
			const auto equal_pos = line.find('=');

			if (equal_pos != std::string::npos)
			{
				std::string url(line.begin()+equal_pos+1, line.end());

				if (!url.empty())
				{
					playlist.push_back(radiotray_ng::trim(url));
				}
			}
		}
	}

	return !playlist.empty();
}
