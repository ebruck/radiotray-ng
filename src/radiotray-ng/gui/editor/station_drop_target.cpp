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
    #include <wx/config.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "editor_app.hpp"
#include "station_drop_target.hpp"


namespace
{
	const int MAX_DRAG_TEXT = 20;
	const int DRAG_FONT_POINT = 12;
	const std::string DRAG_TAIL(" ...");
}

StationDropTarget::StationDropTarget(StationDragAndDrop* object) :
	drag_object(object)
{
}

wxDragResult
StationDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	if (drag_object)
	{
		if (drag_object->onStationDragOver(x, y) == false)
		{
			wxMessageBox(wxT("drag over failed"), wxT("Error"));
		}
	}

	return wxTextDropTarget::OnDragOver(x, y, def);
}

bool
StationDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
	if (drag_object)
	{
		return drag_object->onStationDrop(x, y, data);
	}

	return false;
}

bool
StationDragAndDrop::buildText(const std::string& group, const std::string& station, long item_id, std::string& text)
{
	text = group + TEXT_DELIMITER + station + TEXT_DELIMITER + std::to_string(item_id);
	return true;
}

bool
StationDragAndDrop::extractText(const std::string& text, std::string& group, std::string& station, long& item_id)
{
	std::vector<char> buffer(text.c_str(), text.c_str() + text.size() + 1);

	char* token = std::strtok(&buffer[0], TEXT_DELIMITER.c_str());
	if (token == nullptr)
	{
		return false;
	}
	group = token;

	token = std::strtok(nullptr, TEXT_DELIMITER.c_str());
	if (token == nullptr)
	{
		return false;
	}
	station = token;

	token = std::strtok(nullptr, TEXT_DELIMITER.c_str());
	if (token == nullptr)
	{
		return false;
	}
	item_id = std::stol(token);

	return true;
}

wxBitmap
StationDragAndDrop::makeDragImage(const std::string& text, const wxColour* background, const wxColour* textcolor)
{
	wxBitmap tmp_bitmap(24, 24);
	wxMemoryDC tmp_dc;
	tmp_dc.SelectObject(tmp_bitmap);
	tmp_dc.SetFont(wxFont(DRAG_FONT_POINT, wxSWISS, wxITALIC, wxBOLD));
	int text_width, text_height;
	tmp_dc.GetTextExtent(wxT("u"), &text_width, &text_height);

	int width = MAX_DRAG_TEXT * (text_width - 1);
	int height = text_height * 2;
	wxBitmap bitmap(width, height);
	wxMemoryDC dc;
	dc.SelectObject(bitmap);
	dc.Clear();

	// paint the background
	dc.SetPen(wxPen(*textcolor, 1, wxSOLID));
	dc.SetBrush(wxBrush(*background, wxSOLID));
	dc.DrawRectangle(0, 0, width, height);

	std::string output = text;
	if (output.size() > MAX_DRAG_TEXT)
	{
		output = text.substr(0, MAX_DRAG_TEXT - DRAG_TAIL.size()) + DRAG_TAIL;
	}
	wxString tmpstr(output.c_str(), wxConvUTF8);
	dc.SetFont(wxFont(DRAG_FONT_POINT, wxSWISS, wxITALIC, wxBOLD));
	dc.SetTextForeground(*textcolor);
	dc.DrawText(tmpstr, ((text_width / 2) + 1), (text_height / 2));

	return bitmap;
}

