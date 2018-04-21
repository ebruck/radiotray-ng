// Copyright 2017 Michael A. Burns <michael.burns.oss@gmail.com>
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

#ifndef WX_PRECOMP
    #include <wx/wx.h>
	#include <memory>
#endif


class ImageList final : public wxImageList
{
public:
	ImageList();
	virtual ~ImageList();

	int addImage(const std::string& filename, int default_index);
	int addImage(const wxImage& image);

	bool replaceImage(int index, const std::string& filename);

	std::shared_ptr<wxImageList> getList() { return this->image_list; };

private:
	std::shared_ptr<wxImageList> image_list;
};
