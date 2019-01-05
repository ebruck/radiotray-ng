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
    #include <wx/splitter.h>
	#include <wx/listctrl.h>

	#include <radiotray-ng/common.hpp>
	#include <rtng_user_agent.hpp>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include <wx/aboutdlg.h>

#include "editor_frame.hpp"
#include "editor_app.hpp"
#include "group_list.hpp"
#include "station_list.hpp"

#include <radiotray-ng/config/config.hpp>


// Set to 1 to show the wxWidgets build
// information in the About message box
#define SHOW_WXBUILD_INFO	0


namespace
{
#if SHOW_WXBUILD_INFO
    //helper functions
    enum wxbuildinfoformat
    {
        short_f,
        long_f
    };

    wxString wxbuildinfo(wxbuildinfoformat format)
    {
        wxString wxbuild(wxVERSION_STRING);

        if (format == long_f )
        {
 #if defined(__WXMSW__)
            wxbuild << _T("-Windows");
 #elif defined(__WXMAC__)
            wxbuild << _T("-Mac");
 #elif defined(__UNIX__)
            wxbuild << _T("-Linux");
 #endif

 #if wxUSE_UNICODE
            wxbuild << _T("-Unicode build");
 #else
            wxbuild << _T("-ANSI build");
 #endif // wxUSE_UNICODE
        }

        return wxbuild;
    }
#endif
}

// custom event definition
wxDEFINE_EVENT(SET_BOOKMARKS_DIRTY, wxCommandEvent);

BEGIN_EVENT_TABLE(EditorFrame, wxFrame)
    EVT_CLOSE(EditorFrame::onClose)
    EVT_MENU(idMenuNew, EditorFrame::onNew)
    EVT_MENU(idMenuOpen, EditorFrame::onOpen)
    EVT_MENU(idMenuSave, EditorFrame::onSave)
    EVT_MENU(idMenuSaveAs, EditorFrame::onSaveAs)
    EVT_MENU(idMenuQuit, EditorFrame::onQuit)
    EVT_MENU(idMenuAddGroup, EditorFrame::onAddGroup)
    EVT_MENU(idMenuEditGroup, EditorFrame::onEditGroup)
    EVT_MENU(idMenuCopyGroup, EditorFrame::onCopyGroup)
    EVT_MENU(idMenuDeleteGroup, EditorFrame::onDeleteGroup)
    EVT_MENU(idMenuAddStation, EditorFrame::onAddStation)
    EVT_MENU(idMenuEditStation, EditorFrame::onEditStation)
    EVT_MENU(idMenuCopyStation, EditorFrame::onCopyStation)
    EVT_MENU(idMenuCutStation, EditorFrame::onCutStation)
    EVT_MENU(idMenuPasteStation, EditorFrame::onPasteStation)
    EVT_MENU(idMenuDeleteStation, EditorFrame::onDeleteStation)
    EVT_MENU(idMenuAbout, EditorFrame::onAbout)
    EVT_COMMAND(MAIN_WINDOW_ID, SET_BOOKMARKS_DIRTY, EditorFrame::onSetBookmarksDirty)
	EVT_LIST_ITEM_ACTIVATED(GROUP_LIST_ID, EditorFrame::onGroupActivated)
	EVT_LIST_ITEM_ACTIVATED(STATION_LIST_ID, EditorFrame::onStationActivated)
END_EVENT_TABLE()


EditorFrame::EditorFrame(wxFrame* frame, const wxString& title, const wxString& file_to_load)
    : wxFrame(frame, MAIN_WINDOW_ID, title),
	  splitter(nullptr),
	  group_list(nullptr),
	  station_list(nullptr)
{
	this->createMenus();
	this->createStatusBar();
	this->createAccelerators();
	this->createPanels();
	this->restoreConfiguration();

	if (file_to_load.size() == 0)
	{
		std::string config_path = radiotray_ng::get_data_dir(APP_NAME);
		std::unique_ptr<IConfig> config{std::make_unique<Config>(config_path + RTNG_CONFIG_FILE)};
		if (config->load())
		{
			std::string filename = config->get_string(BOOKMARKS_KEY, RTNG_DEFAULT_BOOKMARK_FILE);
			this->loadBookmarks(filename);
			SetStatusText(filename, 1);
		}
	}
	else
	{
		this->loadBookmarks(file_to_load.ToStdString());
		SetStatusText(file_to_load, 1);
	}
}

EditorFrame::~EditorFrame()
{
	this->saveConfiguration();
}

void
EditorFrame::createMenus()
{
	// create a menu bar
	wxMenuBar* mbar = new wxMenuBar();

	wxMenu* fileMenu = new wxMenu(_T(""));
	fileMenu->Append(idMenuNew, _("&New"), _("Create a new Bookmarks File"));
	fileMenu->Append(idMenuOpen, _("&Open\tCtrl-O"), _("Open a Bookmarks File"));
	fileMenu->Append(idMenuSave, _("&Save\tCtrl-S"), _("Save the Bookmarks File"));
	fileMenu->Append(idMenuSaveAs, _("&Save As"), _("Save the Bookmarks to a new file"));
	fileMenu->AppendSeparator();
	fileMenu->Append(idMenuQuit, _("&Quit\tCtrl-Q"), _("Quit the application"));
	mbar->Append(fileMenu, _("&File"));

	wxMenu* groupMenu = new wxMenu(_T(""));
	groupMenu->Append(idMenuAddGroup, _("&Add"));
	groupMenu->Append(idMenuEditGroup, _("&Edit"));
	groupMenu->Append(idMenuCopyGroup, _("&Copy"));
	groupMenu->Append(idMenuDeleteGroup, _("&Delete"));
	mbar->Append(groupMenu, _("&Group"));

	wxMenu* stationMenu = new wxMenu(_T(""));
	stationMenu->Append(idMenuAddStation, _("&Add\tCtrl-A"));
	stationMenu->Append(idMenuEditStation, _("&Edit\tCtrl-E"));
	stationMenu->Append(idMenuCopyStation, _("&Copy\tCtrl-C"));
	stationMenu->Append(idMenuCutStation, _("Cu&t\tCtrl-X"));
	stationMenu->Append(idMenuPasteStation, _("&Paste\tCtrl-V"));
	stationMenu->Append(idMenuDeleteStation, _("&Delete\tCtrl-D"));
	mbar->Append(stationMenu, _("&Station"));

	wxMenu* helpMenu = new wxMenu(_T(""));
	helpMenu->Append(idMenuAbout, _("&About\tF1"), _("Show info about this application"));
	mbar->Append(helpMenu, _("&Help"));

	this->SetMenuBar(mbar);

	// disable various menu items
	this->GetMenuBar()->Enable(idMenuSave, false);
	this->GetMenuBar()->Enable(idMenuSaveAs, false);
	this->GetMenuBar()->Enable(idMenuAddGroup, false);
	this->GetMenuBar()->Enable(idMenuEditGroup, false);
	this->GetMenuBar()->Enable(idMenuCopyGroup, false);
	this->GetMenuBar()->Enable(idMenuDeleteGroup, false);
	this->GetMenuBar()->Enable(idMenuAddStation, false);
	this->GetMenuBar()->Enable(idMenuEditStation, false);
	this->GetMenuBar()->Enable(idMenuCopyStation, false);
	this->GetMenuBar()->Enable(idMenuCutStation, false);
	this->GetMenuBar()->Enable(idMenuPasteStation, false);
	this->GetMenuBar()->Enable(idMenuDeleteStation, false);
}

void
EditorFrame::createStatusBar()
{
	// create a status bar with some information about the used wxWidgets version
	CreateStatusBar(2);
	SetStatusText(APPLICATION_NAME, 0);
	SetStatusText(wxT("{no file}"), 1);
}

void
EditorFrame::createAccelerators()
{
#define ACCELERATOR_COUNT	9

	// set up accelerator keys
	wxAcceleratorEntry entries[ACCELERATOR_COUNT];

	int count = 0;
	entries[count++].Set(wxACCEL_CTRL, (int) 'O', idMenuOpen);
	entries[count++].Set(wxACCEL_CTRL, (int) 'S', idMenuSave);
	entries[count++].Set(wxACCEL_CTRL, (int) 'Q', idMenuQuit);
	entries[count++].Set(wxACCEL_CTRL, (int) 'A', idMenuAddStation);
	entries[count++].Set(wxACCEL_CTRL, (int) 'E', idMenuEditStation);
	entries[count++].Set(wxACCEL_CTRL, (int) 'C', idMenuCopyStation);
	entries[count++].Set(wxACCEL_CTRL, (int) 'X', idMenuCutStation);
	entries[count++].Set(wxACCEL_CTRL, (int) 'V', idMenuPasteStation);
	entries[count++].Set(wxACCEL_CTRL, (int) 'D', idMenuDeleteStation);

	assert(count == ACCELERATOR_COUNT);

	wxAcceleratorTable accel(ACCELERATOR_COUNT, entries);
	SetAcceleratorTable(accel);
}

void
EditorFrame::createPanels()
{
	this->splitter = new wxSplitterWindow(this);
	this->splitter->SetSashGravity(0.2);

	this->station_list = new StationList(this->splitter);
	this->group_list = new GroupList(this->splitter, this->station_list);

	this->splitter->Initialize(this->group_list);
	this->splitter->SplitVertically(this->group_list, this->station_list);
}

void
EditorFrame::restoreConfiguration()
{
	std::shared_ptr<wxConfig> config = static_cast<EditorApp*>(wxTheApp)->getConfig();

    config->SetPath(wxT("/main"));

    // restore frame position and size
    int x = config->Read(wxT("x"), 50),
        y = config->Read(wxT("y"), 100),
        w = config->Read(wxT("w"), 710),
        h = config->Read(wxT("h"), 420);
    this->Move(x, y);
    this->SetClientSize(w, h);

    // restore split position
    int split = config->Read(wxT("split"), 180);
    this->splitter->SetSashPosition(split);

    // restore window configurations
    this->group_list->restoreConfiguration();
    this->station_list->restoreConfiguration();
}

void
EditorFrame::saveConfiguration()
{
	std::shared_ptr<wxConfig> config = static_cast<EditorApp*>(wxTheApp)->getConfig();

	config->SetPath(wxT("/main"));

	// save the split position
	int split = this->splitter->GetSashPosition();
	config->Write(wxT("split"), (long) split);

	// save the frame position
	int x, y, w, h;
	this->GetClientSize(&w, &h);
	this->GetPosition(&x, &y);
	config->Write(wxT("x"), (long) x);
	config->Write(wxT("y"), (long) y);
	config->Write(wxT("w"), (long) w);
	config->Write(wxT("h"), (long) h);

	// save window configurations
	this->group_list->saveConfiguration();
	this->station_list->saveConfiguration();
}

void
EditorFrame::onNew(wxCommandEvent& /* event */)
{
	if (this->editor_bookmarks.get())
	{
		if (this->editor_bookmarks->isDirty())
		{
			if (this->saveBookmarks(true) == wxCANCEL)
			{
				return;
			}
		}

		this->group_list->clearGroups();
		this->editor_bookmarks.reset();
	}

	this->editor_bookmarks = std::make_shared<EditorBookmarks>("");
	this->group_list->doNew(this->editor_bookmarks);

	this->GetMenuBar()->Enable(idMenuSaveAs, true);
	this->GetMenuBar()->Enable(idMenuAddGroup, true);
	this->GetMenuBar()->Enable(idMenuEditGroup, true);
	this->GetMenuBar()->Enable(idMenuCopyGroup, true);
	this->GetMenuBar()->Enable(idMenuDeleteGroup, true);
	this->GetMenuBar()->Enable(idMenuAddStation, true);
	this->GetMenuBar()->Enable(idMenuEditStation, true);
	this->GetMenuBar()->Enable(idMenuCopyStation, true);
	this->GetMenuBar()->Enable(idMenuCutStation, true);
	this->GetMenuBar()->Enable(idMenuPasteStation, true);
	this->GetMenuBar()->Enable(idMenuDeleteStation, true);
}

void
EditorFrame::onOpen(wxCommandEvent& /* event */)
{
	wxFileDialog dialog(this,
						("Open bookmarks file"),
						"",
						"",
						"Bookmark files (*.json)|*.json",
						wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
	{
		return;
	}

	this->loadBookmarks(dialog.GetPath().ToStdString());
	SetStatusText(dialog.GetPath(), 1);
}

void
EditorFrame::onSave(wxCommandEvent& event)
{
	if (this->editor_bookmarks->getBookmarks()->get_file_name().empty())
	{
		this->onSaveAs(event);
		return;
	}

	this->saveBookmarks();
}

void
EditorFrame::onSaveAs(wxCommandEvent& /* event */)
{
	if (this->editor_bookmarks.get() == nullptr)
	{
		// should never reach here
		wxMessageBox(wxT("No open bookmarks detected!"), wxT("Error"));
		this->GetMenuBar()->Enable(idMenuSave, false);
		this->GetMenuBar()->Enable(idMenuSaveAs, false);
		return;
	}

	wxFileDialog dialog(this,
						wxT("Save bookmarks file"),
						"",
						"",
						"Bookmark files (*.json)|*.json",
						wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK)
	{
		return;
	}

	std::string filename = dialog.GetPath().ToStdString();
	if (filename.find(".json") == std::string::npos)
	{
		filename.append(".json");
	}

	this->saveBookmarks(false, filename);

	this->GetMenuBar()->Enable(idMenuSave, false);
	SetStatusText(filename, 1);
}

void
EditorFrame::onClose(wxCloseEvent& event)
{
	if (this->editor_bookmarks.get() != nullptr && this->editor_bookmarks->isDirty())
	{
		if (this->saveBookmarks(true) == wxCANCEL)
		{
			if (event.CanVeto())
			{
				event.Veto();
				return;
			}
			else
			{
				wxMessageBox(wxT("Unable to veto, application will exit!"), wxT("Warning"));
			}
		}
	}

	this->saveConfiguration();
	this->group_list->clearGroups();
    this->Destroy();
}

void
EditorFrame::onQuit(wxCommandEvent& /* event */)
{
	if (this->editor_bookmarks.get() != nullptr && this->editor_bookmarks->isDirty())
	{
		if (this->saveBookmarks(true) == wxCANCEL)
		{
			return;
		}
	}

	this->saveConfiguration();
	this->group_list->clearGroups();
    this->Destroy();
}

void
EditorFrame::onAddGroup(wxCommandEvent& /* event */)
{
	this->group_list->addGroup();
}

void
EditorFrame::onEditGroup(wxCommandEvent& /* event */)
{
	this->group_list->editGroup();
}

void
EditorFrame::onCopyGroup(wxCommandEvent& /* event */)
{
	this->group_list->copyGroup();
}

void
EditorFrame::onDeleteGroup(wxCommandEvent& /* event */)
{
	this->group_list->deleteGroup();
}

void
EditorFrame::onAddStation(wxCommandEvent& /* event */)
{
	this->station_list->addStation();
}

void
EditorFrame::onEditStation(wxCommandEvent& /* event */)
{
	this->station_list->editStation();
}

void
EditorFrame::onCopyStation(wxCommandEvent& /* event */)
{
	this->station_list->copyStation();
}

void
EditorFrame::onCutStation(wxCommandEvent& /* event */)
{
	this->station_list->cutStation();
}

void
EditorFrame::onPasteStation(wxCommandEvent& /* event */)
{
	this->station_list->pasteStation();
}

void
EditorFrame::onDeleteStation(wxCommandEvent& /* event */)
{
	this->station_list->deleteStation();
}

void
EditorFrame::onAbout(wxCommandEvent& /* event */)
{
    wxAboutDialogInfo aboutInfo;

    std::string image_name(static_cast<EditorApp*>(wxTheApp)->getResourcePath() + RADIOTRAY_NG_LOGO_ICON);
    wxImage image(image_name, wxBITMAP_TYPE_PNG);
    wxBitmap bitmap(image);
	wxIcon icon;
	icon.CopyFromBitmap(bitmap);
	aboutInfo.SetIcon(icon);

	std::string version{APP_NAME_DISPLAY "\nv" RTNG_VERSION};

	// if git version differs, then append hash...
	if (std::string("v" RTNG_VERSION) != std::string(RTNG_GIT_VERSION))
	{
		version += "\n(" RTNG_GIT_VERSION ")";
	}

    std::string name = PROJECT_NAME;
	name += "\n";
    name += APPLICATION_NAME;

    aboutInfo.SetName(APPLICATION_NAME);
	aboutInfo.SetDescription(version);

	std::string license = "Copyright (C) 2017-2019  Michael A. Burns\n\nThis program comes with absolutely no warranty.\n"
		"See the GNU General Public License, version 3 or later for details.";

	aboutInfo.SetCopyright(license);
    aboutInfo.SetWebSite(APP_WEBSITE);

    wxAboutBox(aboutInfo);
}

void
EditorFrame::onSetBookmarksDirty(wxCommandEvent& /* event */)
{
	this->GetMenuBar()->Enable(idMenuSave, true);
}

void
EditorFrame::onGroupActivated(wxListEvent& /* event */)
{
	this->group_list->editGroup();
}

void
EditorFrame::onStationActivated(wxListEvent& /* event */)
{
	this->station_list->editStation();
}

void
EditorFrame::loadBookmarks(const std::string& filename)
{
	if (this->editor_bookmarks.get())
	{
		if (this->editor_bookmarks->isDirty())
		{
			this->saveBookmarks(true);
		}

		this->group_list->clearGroups();
		this->editor_bookmarks.reset();
	}

	LOG(debug) << "loadBookmarks -- " << filename;

	this->editor_bookmarks = std::make_shared<EditorBookmarks>(filename);
	if (this->group_list->loadBookmarks(this->editor_bookmarks) == false)
	{
		wxString msg("A failure occurred loading the bookmarks.");
		wxMessageBox(msg, _("Error"));

		this->editor_bookmarks.reset();
		return;
	}

	this->GetMenuBar()->Enable(idMenuSaveAs, true);
	this->GetMenuBar()->Enable(idMenuAddGroup, true);
	this->GetMenuBar()->Enable(idMenuEditGroup, true);
	this->GetMenuBar()->Enable(idMenuCopyGroup, true);
	this->GetMenuBar()->Enable(idMenuDeleteGroup, true);
	this->GetMenuBar()->Enable(idMenuAddStation, true);
	this->GetMenuBar()->Enable(idMenuEditStation, true);
	this->GetMenuBar()->Enable(idMenuCopyStation, true);
	this->GetMenuBar()->Enable(idMenuPasteStation, true);
	this->GetMenuBar()->Enable(idMenuDeleteStation, true);
}

int
EditorFrame::saveBookmarks(bool ask_to_save, const std::string& file_to_save)
{
	if (this->editor_bookmarks.get() == nullptr)
	{
		// should never reach here
		wxMessageBox(wxT("No open bookmarks detected!"), wxT("Error"));
		this->GetMenuBar()->Enable(idMenuSave, false);
		this->GetMenuBar()->Enable(idMenuSaveAs, false);
		return wxCANCEL;
	}

	if (file_to_save.size() == 0 && this->editor_bookmarks->isDirty() == false)
	{
		// likewise, should never reach here
		this->GetMenuBar()->Enable(idMenuSave, false);
		return wxCANCEL;
	}

	if (ask_to_save)
	{
		int status = wxMessageBox(wxT("Save the Bookmarks?"), wxT("Confirm"), wxYES_NO | wxCANCEL, this);
		if (status != wxYES)
		{
			return status;
		}
	}

	if (!this->editor_bookmarks->getBookmarks()->save_as(file_to_save))
	{
		wxMessageBox("Failed to save the bookmarks!", _("Error"));
		return wxCANCEL;
	}

	this->editor_bookmarks->setDirty(false);

	this->GetMenuBar()->Enable(idMenuSave, false);

	return wxYES;
}
