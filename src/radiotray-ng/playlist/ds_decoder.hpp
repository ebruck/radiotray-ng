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

#include <radiotray-ng/i_playlist_decoder.hpp>


class DsDecoder final : public IPlaylistDecoder
{
public:
    virtual ~DsDecoder() = default;

    bool is_url_direct_stream(const std::string& url);

    bool is_decodable(const std::string& content_type, const std::string& content);

    bool decode(const std::string& content_type, const std::string& payload, playlist_t& playlist);

    std::string get_name();
};
