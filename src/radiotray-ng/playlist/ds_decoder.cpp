// Copyright 2019 Edward G. Bruck <ed.bruck1@gmail.com>
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

#include <radiotray-ng/playlist/ds_decoder.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <algorithm>


std::string DsDecoder::get_name()
{
    return "DsDecoder";
}


bool DsDecoder::is_url_direct_stream(const std::string& url)
{
    std::string tmp{url};

    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

    return boost::ends_with(tmp, ".m3u8") ||
           boost::ends_with(tmp, ".mp3")  ||
           boost::ends_with(tmp, ".aac");
}


bool DsDecoder::is_decodable(const std::string& /*content_type*/, const std::string& /*content*/)
{
    return false;
}


bool DsDecoder::decode(const std::string& /*content_type*/, const std::string& /*content*/, playlist_t& /*playlist*/)
{
    return false;
}
