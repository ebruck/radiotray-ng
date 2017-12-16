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

#include "editor_bookmarks.hpp"
#include "image_list.hpp"
#include "station_drop_target.hpp"
#include "status_dialog.hpp"


#define STATION_LIST_ID		((wxWindowID) 201)


class StationList final : public wxListCtrl, public StationDragAndDrop
{
	DECLARE_DYNAMIC_CLASS(StationList)

public:
	class ItemData
	{
	public:
		ItemData() = delete;
		ItemData(long id, size_t grp_index, size_t sta_index, int img_index) :
			item_id(id),
			group_index(grp_index),
			station_index(sta_index),
			image_index(img_index) {}

		long getItemId() { return this->item_id; }

		long getGroupIndex() { return this->group_index; }

		size_t getStationIndex() { return this->station_index; }
		void setStationIndex(size_t index) { this->station_index = index; }

		int getImageIndex() { return this->image_index; };
		void setImageIndex(int index) { this->image_index = index; };

	private:
		long item_id;
		size_t group_index;
		size_t station_index;
		int image_index;
	};

	StationList();
	StationList(wxWindow* parent);
	virtual ~StationList();

	bool saveConfiguration();
	bool restoreConfiguration();

	void clearStations();
	void setBookmarks(std::shared_ptr<EditorBookmarks> bkm);
	void loadStations(size_t index, const std::string& station_to_select = std::string());
	bool addStation();
	bool editStation();
	bool copyStation();
	bool cutStation();
	bool pasteStation();
	bool deleteStation();

	bool onStationDragOver(wxCoord x, wxCoord y);
	bool onStationDrop(wxCoord x, wxCoord y, const wxString& data);

	void onBeginDrag(wxListEvent& event);
	void onDeleteAllItems(wxListEvent& event);
	void onContextMenu(wxContextMenuEvent& event);
	void onItemRightClick(wxListEvent& event);
	DECLARE_EVENT_TABLE()

private:
	bool isClipboardDataAvailable(bool close_when_done = true);

	size_t group_index;

	int blank_image_index;
	ImageList station_images;

	long last_selected_item_id;
	long drag_item_id;

	std::shared_ptr<EditorBookmarks> editor_bookmarks;
	std::vector<IBookmarks::station_data_t> stations;
};

