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
    #include <wx/config.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "editor_bookmarks.hpp"
#include "editor_frame.hpp"


EditorBookmarks::EditorBookmarks(const std::string& filename) :
	dirty_flag(false)
{
	this->bookmarks = std::make_shared<Bookmarks>(filename);
}

void
EditorBookmarks::setDirty(bool dirty)
{
	this->dirty_flag = dirty;

	// notify the main window
	if (wxTheApp == nullptr || wxTheApp->GetTopWindow() == nullptr)
	{
		return;
	}
	wxWindow* main = wxTheApp->GetTopWindow();

	wxCommandEvent event(SET_BOOKMARKS_DIRTY, main->GetId());
    event.SetEventObject(main);
    event.SetString("dirty");
    main->ProcessWindowEvent(event);
}

