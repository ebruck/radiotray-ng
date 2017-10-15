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
    #include <wx/config.h>
    #include <wx/imaglist.h>
#endif

#include "station_list.hpp"
#include "editor_bookmarks.hpp"
#include "image_list.hpp"
#include "group_drop_target.hpp"
#include "station_drop_target.hpp"


#define GROUP_LIST_ID	((wxWindowID) 101)


class GroupList final : public wxListCtrl, public GroupDragAndDrop, public StationDragAndDrop
{
	DECLARE_DYNAMIC_CLASS(GroupList)

public:
	GroupList();
	GroupList(wxWindow* parent, StationList* list);
	virtual ~GroupList();

	bool saveConfiguration();
	bool restoreConfiguration();

	void clearGroups();
	bool loadBookmarks(std::shared_ptr<EditorBookmarks> bkm);
	bool addGroup();
	bool editGroup();
	bool copyGroup();
	bool deleteGroup();

	bool onGroupDragOver(wxCoord x, wxCoord y);
	bool onGroupDrop(wxCoord x, wxCoord y, const wxString& data);

	bool onStationDragOver(wxCoord x, wxCoord y);
	bool onStationDrop(wxCoord x, wxCoord y, const wxString& data);

private:
    class ItemData
    {
	public:
		ItemData() = delete;
		ItemData(size_t bkm_index, int img_index) : bookmarks_index(bkm_index), image_index(img_index) {};

		size_t getBookmarksIndex() { return this->bookmarks_index; };
		void setBookmarksIndex(size_t index) { this->bookmarks_index = index; };

		int getImageIndex() { return this->image_index; };
		void setImageIndex(int index) { this->image_index = index; };

	private:
		size_t bookmarks_index;
		int image_index;
    };

    void populateList(size_t select_index = static_cast<size_t>(-1));

	void onItemSelected(wxListEvent& event);
	void onBeginDrag(wxListEvent& event);
	void onDeleteAllItems(wxListEvent& event);
	DECLARE_EVENT_TABLE()

    void reindexGroups();

    ImageList group_images;
    int folder_image_index;

    bool have_root;

    long last_selected_item_id;
    long drag_item_id;

	StationList* station_list;

	std::shared_ptr<EditorBookmarks> editor_bookmarks;
};

