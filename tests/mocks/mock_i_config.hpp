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
#include <radiotray-ng/i_config.hpp>

class MockIConfig : public IConfig {
 public:
  MOCK_METHOD0(save,
      bool());
  MOCK_METHOD0(load,
      bool());
  MOCK_METHOD2(set_string,
      void(const std::string& key, const std::string& value));
  MOCK_METHOD2(set_uint32,
      void(const std::string& key, const uint32_t value));
  MOCK_METHOD2(set_bool,
      void(const std::string& key, const bool value));
  MOCK_METHOD2(get_string,
      std::string(const std::string& key, const std::string& default_value));
  MOCK_METHOD2(get_uint32,
      uint32_t(const std::string& key, const uint32_t default_value));
  MOCK_METHOD2(get_bool,
      bool(const std::string& key, const bool default_value));
};
