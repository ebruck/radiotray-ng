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

#include <memory>

class IRadioTrayNG;
class IConfig;
class media_keys_t;


class MediaKeys
{
public:
	MediaKeys(std::shared_ptr<IRadioTrayNG> radiotray_ng, std::shared_ptr<IConfig> config);

private:
	std::shared_ptr<media_keys_t> media_keys_impl;
};
