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
#endif

#include "editor_bookmarks.hpp"


class EditorDialogBase : public wxDialog
{
//	DECLARE_DYNAMIC_CLASS(EditorDialogBase)

public:
	EditorDialogBase();
	virtual ~EditorDialogBase();

	bool create(wxWindow* parent, wxWindowID id, const wxString& title);

	virtual bool createControls() = 0;
	bool addImageButton(wxSizer* image_sizer);
	bool finishDialog(wxSizer* main_sizer);

	virtual std::string getImagePath() = 0;
	virtual bool setImage(const std::string& path) = 0;

	void onBrowseButton(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()

private:
	wxButton* image_button;
};
