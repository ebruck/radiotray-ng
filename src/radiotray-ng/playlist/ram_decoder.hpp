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

#include "m3u_decoder.hpp"


// RAM is the same as m3u
class RamDecoder final : public M3uDecoder
{
public:
	virtual ~RamDecoder() = default;

	bool is_url_direct_stream(const std::string& /*url*/, const std::string& /*content_type*/) { return false; };

	bool is_decodable(const std::string& content_type, const std::string& content);

	std::string get_name();
};
