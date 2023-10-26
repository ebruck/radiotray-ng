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

#include <radiotray-ng/common.hpp>
#include <radiotray-ng/i_playlist_decoder.hpp>
#include <radiotray-ng/i_config.hpp>
#include <rtng_user_agent.hpp>
#include <radiotray-ng/playlist/playlist_downloader.hpp>
#include "asf_decoder.hpp"
#include "asx_decoder.hpp"
#include "m3u_decoder.hpp"
#include "ds_decoder.hpp"
#include "pls_decoder.hpp"
#include "ram_decoder.hpp"
#include "xspf_decoder.hpp"
#include <curl/curl.h>


PlaylistDownloader::PlaylistDownloader(std::shared_ptr<IConfig> config)
	: config(std::move(config))
{
	this->install_decoders();
}


void PlaylistDownloader::install_decoders()
{
	this->decoders.emplace_back(std::make_shared<DsDecoder>());
	this->decoders.emplace_back(std::make_shared<PlsDecoder>());
	this->decoders.emplace_back(std::make_shared<M3uDecoder>());
	this->decoders.emplace_back(std::make_shared<AsxDecoder>());
	this->decoders.emplace_back(std::make_shared<RamDecoder>());
	this->decoders.emplace_back(std::make_shared<AsfDecoder>());
	this->decoders.emplace_back(std::make_shared<XspfDecoder>());
}


bool PlaylistDownloader::is_url_direct_stream(const std::string& url)
{
	for(auto& decoder : this->decoders)
	{
		if (decoder->is_url_direct_stream(url))
		{
			LOG(info) << "detected as a direct stream, decoder: " << decoder->get_name();
			return true;
		}
	}
	return false;
}


bool PlaylistDownloader::download_playlist(const IBookmarks::station_data_t& std, playlist_t& playlist)
{
	playlist.clear();

	std::string content;
	std::string content_type;

	long http_resp_code{0};

	if (std.direct || this->is_url_direct_stream(std.url))
	{
		if (std.direct)
		{
			LOG(info) << "detected as a direct stream: direct=true";
		}

		playlist.push_back(std.url);
		return true;
	}

	// Try downloading N bytes in case it's a media stream
	std::string redirected_url;
	if (!this->download(std.url, redirected_url, content_type, content, http_resp_code, 4096))
	{
		LOG(error) << "Could not download playlist!";

		if (http_resp_code != HTTP_NOT_FOUND && http_resp_code != 0)
		{
			// Must be a media stream?
			LOG(info) << "decoder: none";
			playlist.push_back(std.url);
			return true;
		}

		LOG(error) << "http_resp_code = " << http_resp_code;

		return false;
	}

	// lets see if redirected url is a direct stream...
	if (!redirected_url.empty())
	{
		if (this->is_url_direct_stream(redirected_url))
		{
			playlist.push_back(redirected_url);
			return true;
		}
	}

	// lets see what this is...
	auto decoder = this->inspect(content_type, content);

	if (decoder)
	{
		// A decoder has identified it, so lets extract the playlist...
		LOG(info) << "decoder: " << decoder->get_name() << ", " << content_type;

		return decoder->decode(content_type, content, playlist);
	}

	LOG(info) << "no decoders, assuming direct media stream of content-type: " << content_type;

	playlist.push_back(std.url);

	return true;
}


bool PlaylistDownloader::download(const std::string& url, std::string& redirected_url, std::string& content_type, std::string& content, long& http_resp_code, size_t max_bytes)
{
	std::unique_ptr<CURL, std::function<void(CURL*)>> curl_handle(curl_easy_init(),
		std::bind(curl_easy_cleanup, std::placeholders::_1));

	content.clear();

	if (curl_handle)
	{
		// handle icecast's "ICY 200 OK"
		curl_slist* http_200_aliases{nullptr};
		http_200_aliases = curl_slist_append(http_200_aliases, "ICY");

		if (http_200_aliases != nullptr)
		{
			curl_easy_setopt(curl_handle.get(), CURLOPT_HTTP200ALIASES, http_200_aliases);
		}
		else
		{
			LOG(error) << "failed to allocate http 200 alias list";
		}

		// defaults
		curl_easy_setopt(curl_handle.get(), CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_handle.get(), CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_handle.get(), CURLOPT_FAILONERROR, 1L);

		curl_easy_setopt(curl_handle.get(), CURLOPT_SSL_VERIFYHOST, 0);

		const uint32_t http_timeout = this->config->get_uint32(HTTP_TIMEOUT_KEY, DEFAULT_HTTP_TIMEOUT_VALUE);

		LOG(debug) << HTTP_TIMEOUT_KEY << "=" << http_timeout;

		curl_easy_setopt(curl_handle.get(), CURLOPT_TIMEOUT, http_timeout);
		curl_easy_setopt(curl_handle.get(), CURLOPT_WRITEFUNCTION,
			reinterpret_cast<void*>(PlaylistDownloader::curl_write_callback));
		curl_easy_setopt(curl_handle.get(), CURLOPT_USERAGENT, RTNG_USER_AGENT);

		callback_pair_t cb_pair{&content, max_bytes};
		curl_easy_setopt(curl_handle.get(), CURLOPT_WRITEDATA, static_cast<void*>(&cb_pair));

		CURLcode res = curl_easy_perform(curl_handle.get());

		curl_slist_free_all(http_200_aliases);
		curl_easy_getinfo(curl_handle.get(), CURLINFO_RESPONSE_CODE, &http_resp_code);

		// write error usually means we hit our upper limit for data to retrieve..
		if (res != CURLE_OK && res != CURLE_WRITE_ERROR)
		{
			LOG(error) << "curl_easy_perform() failed: " << curl_easy_strerror(res) << " " << http_resp_code;
			return false;
		}

		// get content-type
		char* c_type = nullptr;
		curl_easy_getinfo(curl_handle.get(), CURLINFO_CONTENT_TYPE, &c_type);

		if (c_type)
		{
			content_type = c_type;

			char* effective_url = nullptr;
			curl_easy_getinfo(curl_handle.get(), CURLINFO_EFFECTIVE_URL, &effective_url);
			if (effective_url)
			{
				if (std::string(effective_url) != url)
				{
					redirected_url = effective_url;
				}
			}
        }

		// probably a stream?
		if (content.size() < max_bytes)
		{
			LOG(debug) << std::endl << content;
		}
	}

	return !content.empty();
}


size_t PlaylistDownloader::curl_write_callback(char* buffer, size_t size, size_t nitems, void* userdata)
{
	size_t data_size = size * nitems;

	auto cb_pair = reinterpret_cast<callback_pair_t*>(userdata);
	cb_pair->first->append(buffer, data_size);

	// should we read all of it?
	if (cb_pair->second == 0)
	{
		return data_size;
	}

	// we only want to read up to a specified size...
	if (cb_pair->first->size() >= cb_pair->second)
	{
		// stop write operation...
		LOG(warning) << "Content exceeds our limit of " << cb_pair->second << " bytes!";
		return 0;
	}

	return data_size;
}


std::shared_ptr<IPlaylistDecoder> PlaylistDownloader::inspect(const std::string& content_type, const std::string& content)
{
	for(auto& decoder : decoders)
	{
		if (decoder->is_decodable(content_type, content))
		{
			return decoder;
		}
	}

	return nullptr;
}
