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

#include <radiotray-ng/playlist/asf_decoder.hpp>
#include <radiotray-ng/string_helpers.hpp>
#include <regex>
#include <sstream>


std::string AsfDecoder::get_name()
{
	return "ASF";
}


bool AsfDecoder::is_decodable(const std::string& content_type, const std::string& content)
{
	if (content_type.find("video/x-ms-asf") != std::string::npos &&
	   (content.find("[reference]")         != std::string::npos ||
	    content.find("[Reference]")         != std::string::npos))
	{
		return true;
	}

	return false;
}


bool AsfDecoder::decode(const std::string& content, playlist_t& playlist)
{
	if (content.empty())
	{
		return false;
	}

	playlist.clear();

	std::stringstream ss(content);
	const std::string ref_entry = "Ref";

	char line_terminator = radiotray_ng::guess_line_terminator(content);

	std::string line;
	while(std::getline(ss, line, line_terminator))
	{
		if (line.compare(0, ref_entry.length(), ref_entry) == 0)
		{
			const auto equal_pos = line.find("=");

			if (equal_pos != std::string::npos)
			{
				std::string url(line.begin()+equal_pos+1, line.end());

				if (!url.empty())
				{
					if (url.rfind("?MSWMExt=.asf") != std::string::npos)
					{
						auto it = url.find("http");

						if (it != std::string::npos)
						{
							url.replace(it, 4, "mms");
						}
					}

					playlist.push_back(radiotray_ng::trim(url));
				}
			}
		}
	}

	return !playlist.empty();
}
