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
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "editor_app.hpp"
#include "group_drop_target.hpp"


namespace
{
}

GroupDropTarget::GroupDropTarget(GroupDragAndDrop* object) :
	drag_object(object)
{
}

wxDragResult
GroupDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	if (drag_object)
	{
		if (drag_object->onGroupDragOver(x, y) == false)
		{
			wxMessageBox(wxT("drag over failed"), wxT("Error"));
		}
	}

	return wxTextDropTarget::OnDragOver(x, y, def);
}

bool
GroupDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
	if (drag_object)
	{
		return drag_object->onGroupDrop(x, y, data);
	}

	return false;
}

bool
GroupDragAndDrop::buildText(const std::string& group, long item_id, std::string& text)
{
	text = group + TEXT_DELIMITER + std::to_string(item_id);
	return true;
}

bool
GroupDragAndDrop::extractText(const std::string& text, std::string& group, long& item_id)
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
	item_id = std::stol(token);

	return true;
}

