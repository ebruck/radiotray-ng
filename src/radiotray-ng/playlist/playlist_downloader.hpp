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

#include <radiotray-ng/i_playlist_downloader.hpp>
#include <memory>

class IPlaylistDecoder;
class IConfig;


class PlaylistDownloader final : public IPlaylistDownloader
{
public:

	PlaylistDownloader(std::shared_ptr<IConfig> config);

	virtual ~PlaylistDownloader() = default;

	/**
	 * Downloads playlist and runs the output through the list of supported decoders.
	 *
	 * @param std      station data
	 * @param playlist playlist
	 * @return true if download and parsing was successful
	 */
	bool download_playlist(const IBookmarks::station_data_t& std, playlist_t& playlist);

private:
	std::shared_ptr<IPlaylistDecoder> inspect(const std::string& content_type, const std::string& content);

	bool download(const std::string& url, std::string& redirected_url, std::string& content_type, std::string& content, long& http_resp_code, size_t max_bytes);

	void install_decoders();

	bool is_url_direct_stream(const std::string& url);

	using callback_pair_t = std::pair<std::string*, size_t>;

	static size_t curl_write_callback(char *buffer, size_t size, size_t nitems, void* userdata);

	std::vector<std::shared_ptr<IPlaylistDecoder>> decoders;
	std::shared_ptr<IConfig> const config;
};
