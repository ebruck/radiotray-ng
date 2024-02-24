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
    #include <wx/splitter.h>
	#include <wx/listctrl.h>
    #include <memory>
#endif

#include "group_list.hpp"
#include "station_list.hpp"

#include <radiotray-ng/bookmarks/bookmarks.hpp>


// define a window id
#define MAIN_WINDOW_ID			5001

// set up a custom event for handling a change to the Bookmarks data
wxDECLARE_EVENT(SET_BOOKMARKS_DIRTY, wxCommandEvent);


// define the main window
class EditorFrame : public wxFrame
{
public:
    EditorFrame(wxFrame* frame, const wxString& title, const wxString& filename);
    virtual ~EditorFrame();

    enum
    {
        idMenuQuit = 1000,
        idMenuNew,
        idMenuOpen,
        idMenuSave,
        idMenuSaveAs,
        idMenuAddGroup,
        idMenuEditGroup,
        idMenuCopyGroup,
        idMenuDeleteGroup,
        idMenuAddStation,
        idMenuEditStation,
        idMenuCopyStation,
        idMenuCutStation,
        idMenuPasteStation,
        idMenuDeleteStation,
        idMenuAbout
    };

private:
    void enableMenus();

	void onNew(wxCommandEvent& event);
    void onOpen(wxCommandEvent& event);
    void onSave(wxCommandEvent& event);
    void onSaveAs(wxCommandEvent& event);
    void onClose(wxCloseEvent& event);
    void onQuit(wxCommandEvent& event);

    void onAddGroup(wxCommandEvent& event);
    void onEditGroup(wxCommandEvent& event);
    void onCopyGroup(wxCommandEvent& event);
    void onDeleteGroup(wxCommandEvent& event);

    void onAddStation(wxCommandEvent& event);
    void onEditStation(wxCommandEvent& event);
    void onCopyStation(wxCommandEvent& event);
    void onCutStation(wxCommandEvent& event);
    void onPasteStation(wxCommandEvent& event);
    void onDeleteStation(wxCommandEvent& event);

    void onAbout(wxCommandEvent& event);

    void onSetBookmarksDirty(wxCommandEvent& event);

    void onGroupActivated(wxListEvent& event);
    void onStationActivated(wxListEvent& event);

    DECLARE_EVENT_TABLE()

    void createMenus();
    void createStatusBar();
    void createAccelerators();
    void createPanels();
    void restoreConfiguration();
    void saveConfiguration();

    void loadBookmarks(const std::string& filename, bool create_if_nonexistent = false);
    int saveBookmarks(bool ask_to_save = false, const std::string& file_to_save = std::string());

    wxSplitterWindow* splitter;
    GroupList* group_list;
    StationList* station_list;

    std::shared_ptr<EditorBookmarks> editor_bookmarks;
};

