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

#include "editor_dialog_base.hpp"
#include "editor_bookmarks.hpp"


class StationEditorDialog : public EditorDialogBase
{
//	DECLARE_DYNAMIC_CLASS(StationEditorDialog)

public:
	StationEditorDialog();
	StationEditorDialog(wxWindow* parent);
	virtual ~StationEditorDialog();

	virtual bool createControls();
	virtual std::string getImagePath();
	virtual bool setImage(const std::string& path);

	void setData(const std::string& name, const std::string& url, const std::string& image, const bool notifications);
	void getData(std::string& name, std::string& url, std::string& image, bool& notifications);

	DECLARE_EVENT_TABLE()

private:
	wxTextCtrl* name_control;
	wxTextCtrl* url_control;
	wxStaticBitmap* bitmap_control;
	wxTextCtrl* image_control;
	wxCheckBox* notify_control;
};
