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

#ifdef WX_PRECOMP
	#include "wx_pch.hpp"
#else
    #include <wx/imaglist.h>
    #include <wx/filename.h>
	#include <memory>

	#include <radiotray-ng/common.hpp>
	#include <radiotray-ng/helpers.hpp>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "image_list.hpp"


namespace
{
#if defined(WINDOWS)
	const int IMAGE_WIDTH_DEFAULT = 16;
	const int IMAGE_HEIGHT_DEFAULT = 16;
#else
	const int IMAGE_WIDTH_DEFAULT = 24;
	const int IMAGE_HEIGHT_DEFAULT = 24;
#endif
}


ImageList::ImageList()
{
	this->image_list = std::make_shared<wxImageList>();
}

ImageList::~ImageList()
{
	//dtor
}

int
ImageList::addImage(const std::string& filename, int default_index)
{
	if (this->image_list.get() == nullptr)
	{
		return default_index;
	}

	wxFileName image_file(radiotray_ng::word_expand(filename));
	if (image_file.Exists() == false || image_file.IsFileReadable() == false)
	{
		return default_index;
	}

	/// @todo The following is a "temporary" workaround for the
	///       issue identified in 15331. This can be removed once
	///       wxWidgets 3.1 is available.
	///
	///       http://trac.wxwidgets.org/ticket/15331
	wxLogNull log_null;

	return this->addImage(wxImage(image_file.GetFullPath()));
}

int
ImageList::addImage(const wxImage& image)
{
	/// @todo The following is a "temporary" workaround for the
	///       issue identified in 15331. This can be removed once
	///       wxWidgets 3.1 is available.
	///
	///       http://trac.wxwidgets.org/ticket/15331
	wxLogNull log_null;

	return image_list->Add(wxBitmap(image.Scale(IMAGE_WIDTH_DEFAULT, IMAGE_HEIGHT_DEFAULT)));
}

bool
ImageList::replaceImage(int index, const std::string& filename)
{
	wxFileName image_file(radiotray_ng::word_expand(filename));
	if (image_file.Exists() == false || image_file.IsFileReadable() == false)
	{
		return false;
	}

	/// @todo The following is a "temporary" workaround for the
	///       issue identified in 15331. This can be removed once
	///       wxWidgets 3.1 is available.
	///
	///       http://trac.wxwidgets.org/ticket/15331
	wxLogNull log_null;

	return image_list->Replace(index, wxBitmap(wxImage(radiotray_ng::word_expand(filename)).Scale(IMAGE_WIDTH_DEFAULT, IMAGE_HEIGHT_DEFAULT)));
}

