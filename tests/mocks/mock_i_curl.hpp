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

#include <gmock/gmock.h>
#include <radiotray-ng/i_curl.hpp>

class MockICurl : public ICurl {
 public:
  MOCK_METHOD0(curl_easy_init,
      CURL*());
  MOCK_METHOD3(curl_easy_setopt,
      CURLcode(CURL* curl, CURLoption option, const std::string& param));
  MOCK_METHOD3(curl_easy_setopt,
      CURLcode(CURL* curl, CURLoption option, const long param));
  MOCK_METHOD3(curl_easy_setopt,
      CURLcode(CURL* curl, CURLoption option, void* param));
  MOCK_METHOD3(curl_easy_getinfo,
      CURLcode(CURL* curl, CURLINFO option, void* param));
  MOCK_METHOD1(curl_easy_perform,
      CURLcode(CURL* curl));
  MOCK_METHOD1(curl_easy_strerror,
	  const char*(CURLcode error));
  MOCK_METHOD1(curl_easy_cleanup,
      void(CURL* curl));
  MOCK_METHOD2(curl_slist_append,
	  curl_slist*(curl_slist* list, const char* string));
  MOCK_METHOD1(curl_slist_free_all,
	  void(curl_slist* list));
};
