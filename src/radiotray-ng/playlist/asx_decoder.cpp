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

#include <radiotray-ng/playlist/asx_decoder.hpp>
#include <radiotray-ng/helpers.hpp>
#include <regex>


std::string AsxDecoder::get_name()
{
	return "ASX";
}


bool AsxDecoder::is_decodable(const std::string& content_type, const std::string& content)
{
	if ((content_type.find("video/x-ms-wvx") != std::string::npos  ||
	     content_type.find("video/x-ms-asf") != std::string::npos  ||
	     content_type.find("video/x-ms-wmv") != std::string::npos) &&
	    (content.find("<asx")                != std::string::npos  ||
	     content.find("<ASX")                != std::string::npos))
	{
		return true;
	}

	return false;
}


bool AsxDecoder::decode(const std::string& content, playlist_t& playlist)
{
	if (content.empty())
	{
		return false;
	}

	playlist.clear();

	std::regex  regex("href[ ]*=[ \"']*(.*?)[ \"']", std::regex::icase);
	std::smatch match;	
	std::string search{content};
	
	while (std::regex_search(search, match, regex))
	{
		std::string url{match[1]};
		playlist.push_back(radiotray_ng::trim(url));
	    search = match.suffix().str();
	}

	return !playlist.empty();
}
