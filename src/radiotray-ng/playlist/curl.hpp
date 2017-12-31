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

#include <radiotray-ng/i_curl.hpp>


class Curl final : public ICurl
{
public:
	Curl();

	virtual ~Curl();

	CURL* curl_easy_init();

	CURLcode curl_easy_setopt(CURL* curl, CURLoption option, const std::string& param);

	CURLcode curl_easy_setopt(CURL* curl, CURLoption option, long param);

	CURLcode curl_easy_setopt(CURL* curl, CURLoption option, void* param);

	CURLcode curl_easy_getinfo(CURL *curl, CURLINFO info, void* param);

	CURLcode curl_easy_perform(CURL* curl);

	const char* curl_easy_strerror(CURLcode error);

	void curl_easy_cleanup(CURL* curl);

	curl_slist* curl_slist_append(curl_slist* list, const char* string);

	void curl_slist_free_all(curl_slist* list);
};
