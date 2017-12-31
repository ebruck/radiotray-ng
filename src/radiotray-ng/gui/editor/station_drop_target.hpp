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
    #include <wx/dnd.h>
#endif


class StationDragAndDrop
{
public:
	virtual ~StationDragAndDrop() = default;

	virtual bool onStationDragOver(wxCoord x, wxCoord y) = 0;
	virtual bool onStationDrop(wxCoord x, wxCoord y, const wxString& data) = 0;

	bool buildText(const std::string& group, const std::string& station, long item_id, std::string& text);
	bool extractText(const std::string& text, std::string& group, std::string& station, long& item_id);
	wxBitmap makeDragImage(const std::string& text, const wxColour* background = wxLIGHT_GREY, const wxColour* textcolor = wxBLACK);

protected:
	StationDragAndDrop() = default;
};


class StationDropTarget : public wxTextDropTarget
{
public:
	StationDropTarget() = delete;
	StationDropTarget(StationDragAndDrop* object);
	virtual ~StationDropTarget() = default;

	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
	virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);

private:
	StationDragAndDrop*	drag_object;
};
