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

#include <radiotray-ng/playlist/curl.hpp>


Curl::Curl()
{
	::curl_global_init(CURL_GLOBAL_ALL);
}

Curl::~Curl()
{
	::curl_global_cleanup();
}


CURL* Curl::curl_easy_init()
{
	return ::curl_easy_init();
}


CURLcode Curl::curl_easy_setopt(CURL* curl, CURLoption option, const std::string& param)
{
	return ::curl_easy_setopt(curl, option, param.c_str());
}


CURLcode Curl::curl_easy_setopt(CURL* curl, CURLoption option, const long param)
{
	return ::curl_easy_setopt(curl, option, param);
}

CURLcode Curl::curl_easy_setopt(CURL* curl, CURLoption option, void* param)
{
	return ::curl_easy_setopt(curl, option, param);
}


CURLcode Curl::curl_easy_getinfo(CURL *curl, CURLINFO info, void* param)
{
	return ::curl_easy_getinfo(curl, info, param);
}


CURLcode Curl::curl_easy_perform(CURL* curl)
{
	return ::curl_easy_perform(curl);
}


const char* Curl::curl_easy_strerror(CURLcode error)
{
	return ::curl_easy_strerror(error);
}


void Curl::curl_easy_cleanup(CURL* curl)
{
	::curl_easy_cleanup(curl);
}


curl_slist* Curl::curl_slist_append(curl_slist* list, const char* string)
{
	return ::curl_slist_append(list, string);
}


void Curl::curl_slist_free_all(curl_slist* list)
{
	::curl_slist_free_all(list);
}
