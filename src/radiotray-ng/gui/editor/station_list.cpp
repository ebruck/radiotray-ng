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
	#include <wx/listctrl.h>
    #include <wx/imaglist.h>
    #include <wx/config.h>
	#include <wx/clipbrd.h>

	#include <radiotray-ng/helpers.hpp>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "editor_app.hpp"
#include "editor_frame.hpp"
#include "station_list.hpp"
#include "station_editor_dialog.hpp"

#include "images/blank.xpm"
#include "images/move-icon.xpm"


IMPLEMENT_DYNAMIC_CLASS(StationList, wxListCtrl)


BEGIN_EVENT_TABLE(StationList, wxListCtrl)
	EVT_LIST_DELETE_ALL_ITEMS(STATION_LIST_ID, StationList::onDeleteAllItems)
	EVT_LIST_BEGIN_DRAG(STATION_LIST_ID, StationList::onBeginDrag)
	EVT_LIST_ITEM_RIGHT_CLICK(STATION_LIST_ID, StationList::onItemRightClick)
END_EVENT_TABLE()


namespace
{
	const wxString NAME_COLUMN_TEXT(wxT("Name"));
	const long NAME_COLUMN_INDEX = 0;
	const int NAME_COLUMN_WIDTH = 160;

	const wxString URL_COLUMN_TEXT(wxT("URL"));
	const long URL_COLUMN_INDEX = 1;
	const int URL_COLUMN_WIDTH = 340;

	const wxString NOTIFICATION_COLUMN_TEXT(wxT("Notify"));
	const long NOTIFICATION_COLUMN_INDEX = 2;
	const int NOTIFICATION_COLUMN_WIDTH = 60;

	const int SCROLL_POSITION_DIVISOR = 3;

	const wxString CONFIG_PATH(wxT("/station"));
	const wxString CONFIG_COLUMN_1(wxT("col1"));
	const wxString CONFIG_COLUMN_2(wxT("col2"));
	const wxString CONFIG_COLUMN_3(wxT("col3"));
}


StationList::StationList() :
	group_index(-1),
	blank_image_index(-1),
	last_selected_item_id(0),
	drag_item_id(0)
{
}

StationList::StationList(wxWindow* parent) : wxListCtrl(parent, STATION_LIST_ID),
	group_index(-1),
	last_selected_item_id(0),
	drag_item_id(0)
{
	long style = wxLC_REPORT | wxLC_SINGLE_SEL;
	this->SetWindowStyle(style);

	// create headers
	this->InsertColumn(NAME_COLUMN_INDEX, NAME_COLUMN_TEXT);
	this->InsertColumn(URL_COLUMN_INDEX, URL_COLUMN_TEXT);
	this->InsertColumn(NOTIFICATION_COLUMN_INDEX, NOTIFICATION_COLUMN_TEXT);

	this->blank_image_index = this->station_images.addImage(wxImage(blank_xpm));
	this->SetImageList(this->station_images.getList().get(), wxIMAGE_LIST_SMALL);

	this->SetDropTarget(new StationDropTarget(this));
}

StationList::~StationList()
{
}

bool
StationList::saveConfiguration()
{
	std::shared_ptr<wxConfig> config = static_cast<EditorApp*>(wxTheApp)->getConfig();

	config->SetPath(CONFIG_PATH);
	config->Write(CONFIG_COLUMN_1, this->GetColumnWidth(0));
	config->Write(CONFIG_COLUMN_2, this->GetColumnWidth(1));
	config->Write(CONFIG_COLUMN_3, this->GetColumnWidth(2));

	return true;
}

bool
StationList::restoreConfiguration()
{
	std::shared_ptr<wxConfig> config = static_cast<EditorApp*>(wxTheApp)->getConfig();

	config->SetPath(CONFIG_PATH);

    int col1 = config->Read(CONFIG_COLUMN_1, NAME_COLUMN_WIDTH);
    this->SetColumnWidth(NAME_COLUMN_INDEX, col1);

    int col2 = config->Read(CONFIG_COLUMN_2, URL_COLUMN_WIDTH);
    this->SetColumnWidth(URL_COLUMN_INDEX, col2);

    int col3 = config->Read(CONFIG_COLUMN_3, NOTIFICATION_COLUMN_WIDTH);
    this->SetColumnWidth(NOTIFICATION_COLUMN_INDEX, col3);

	return true;
}

void
StationList::onDeleteAllItems(wxListEvent& /* event */)
{
	long item = -1;
	for ( ;; )
	{
		item = this->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
		if (item == -1)
		{
			break;
		}

		StationList::ItemData* data = reinterpret_cast<StationList::ItemData*>(this->GetItemData(item));
		if (data)
		{
			delete data;
		}
	}

	this->stations.clear();
}

void
StationList::clearStations()
{
	this->DeleteAllItems();
}

void
StationList::setBookmarks(std::shared_ptr<EditorBookmarks> bkm)
{
	if (this->editor_bookmarks.get())
	{
		this->clearStations();
		this->editor_bookmarks.reset();
	}

	this->editor_bookmarks = bkm;
}

void
StationList::loadStations(size_t index, const std::string& station_to_select)
{
	if (this->editor_bookmarks.get() == nullptr)
	{
		wxString msg = "Bookmarks is not set, cannot load stations!";
		wxMessageBox(msg, _("Error"));
		return;
	}

	this->clearStations();
	this->group_index = index;

	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[this->group_index];
	if (this->editor_bookmarks->getBookmarks()->get_group_stations(group.group, this->stations) == false)
	{
		wxString msg = "Failed to retrieve the stations, aborting load";
		wxMessageBox(msg, _("Error"));
		return;
	}

	// use traditional for loop for access to index position
	long item_to_select = -1;
	for (size_t station_index = 0; station_index < this->stations.size(); station_index++)
	{
		wxString tmpstr(this->stations[station_index].name.c_str(), wxConvUTF8);

		int use_image_index = this->station_images.addImage(this->stations[station_index].image, this->blank_image_index);

		long item_id = this->InsertItem(this->GetItemCount(), tmpstr, use_image_index);
		this->SetItem(item_id, URL_COLUMN_INDEX, this->stations[station_index].url);
		this->setNotify(item_id, this->stations[station_index].notifications);
		this->SetItemPtrData(item_id, reinterpret_cast<wxUIntPtr>(new StationList::ItemData(item_id, this->group_index, station_index, use_image_index)));

		if (station_to_select.compare(this->stations[station_index].name) == 0)
		{
			item_to_select = item_id;
		}
	}

	if (item_to_select != -1)
	{
		this->SetItemState(item_to_select, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		this->EnsureVisible(item_to_select);
	}
}

bool
StationList::addStation()
{
	StationEditorDialog dlg(this->GetParent());
	dlg.setData("", "", "", true); // set default notify to "on"

	if (dlg.ShowModal() != wxID_OK)
	{
		dlg.Destroy();
		return true;
	}

	std::string name, url, image;
	bool notifications;
	dlg.getData(name, url, image, notifications);
	dlg.Destroy();

	// trim data
	name = radiotray_ng::trim(name);
	url = radiotray_ng::trim(url);
	image = radiotray_ng::trim(image);

	if (name.size() == 0 || url.size() == 0)
	{
		return false;
	}

	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[this->group_index];
	if (this->editor_bookmarks->getBookmarks()->add_station(group.group, name, url, image, true /* todo: get value from editor? */) == false)
	{
		wxMessageBox(wxT("Failed to add the station."), wxT("Error"));
		return false;
	}
	this->editor_bookmarks->setDirty();

	// update vector
	IBookmarks::station_data_t station_data;
	station_data.name = name;
	station_data.url = url;
	station_data.notifications = notifications;
	wxFileName image_file(radiotray_ng::word_expand(image));
	if (image_file.Exists() && image_file.IsFileReadable())
	{
		station_data.image = image;
	}
	this->stations.push_back(station_data);
	int station_index = this->stations.size() - 1;
	if (name.compare(this->stations[station_index].name) != 0)
	{
		std::string msg = "Invalid index - " + name + " != " + this->stations[station_index].name + " (" + std::to_string(station_index);
		wxMessageBox(msg, wxT("Error"));
	}

	int image_index = this->station_images.addImage(image, this->blank_image_index);

	// add item
	wxString tmpstr(name.c_str(), wxConvUTF8);
	long item_id = this->InsertItem(this->GetItemCount(), tmpstr, image_index);
	this->SetItem(item_id, URL_COLUMN_INDEX, url);
	this->setNotify(item_id, notifications);
	this->SetItemPtrData(item_id, reinterpret_cast<wxUIntPtr>(new StationList::ItemData(item_id, this->group_index, station_index, image_index)));
	this->SetItemState(item_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	return true;
}

bool
StationList::editStation()
{
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return false;
	}

	StationList::ItemData* data = reinterpret_cast<StationList::ItemData*>(this->GetItemData(item));
	wxString tmpstr = this->GetItemText(item, NAME_COLUMN_INDEX);
	std::string original_name = std::string(tmpstr.mb_str(wxConvUTF8));
	std::string original_url = this->GetItemText(item, URL_COLUMN_INDEX).ToStdString();
	std::string original_image = this->stations[data->getStationIndex()].image;
	bool original_notifications = this->stations[data->getStationIndex()].notifications;

	StationEditorDialog dlg(this->GetParent());
	dlg.setData(original_name, original_url, original_image, original_notifications);
	if (dlg.ShowModal() != wxID_OK)
	{
		dlg.Destroy();
		return true;
	}

	std::string name, url, image;
	bool notifications;
	dlg.getData(name, url, image, notifications);
	dlg.Destroy();

	// trim data
	name = radiotray_ng::trim(name);
	url = radiotray_ng::trim(url);
	image = radiotray_ng::trim(image);

	if (name.compare(original_name) == 0 &&
		url.compare(original_url) == 0 &&
		image.compare(original_image) == 0 &&
		notifications == original_notifications)
	{
		// no changes, so bail
		return true;
	}

	if (!image.empty())
	{
		wxFileName image_file(radiotray_ng::word_expand(image));
		if (image_file.Exists() == false || image_file.IsFileReadable() == false)
		{
			image = original_image;
		}
	}

	// verify image file
	// update data
	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[this->group_index];
	if (name.compare(original_name) != 0)
	{
		if (this->editor_bookmarks->getBookmarks()->rename_station(group.group, original_name, name))
		{
			this->editor_bookmarks->setDirty();
		}
	}
	if (url.compare(original_url) != 0 || image.compare(original_image) != 0 || notifications != original_notifications)
	{
		if (this->editor_bookmarks->getBookmarks()->update_station(group.group, name, url, image, notifications))
		{
			this->editor_bookmarks->setDirty();
		}
	}

	// update vector
	this->stations[data->getStationIndex()].name = name;
	this->stations[data->getStationIndex()].url = url;
	this->stations[data->getStationIndex()].image = image;
	this->stations[data->getStationIndex()].notifications = notifications;

	// update list data
	tmpstr = wxString(name.c_str(), wxConvUTF8);
	this->SetItem(item, NAME_COLUMN_INDEX, tmpstr);
	this->SetItem(item, URL_COLUMN_INDEX, wxString(url));
	this->setNotify(item, notifications);

	if (image.compare(original_image) != 0)
	{
		if (image.size() == 0)
		{
			data->setImageIndex(this->blank_image_index);
		}
		else
		{
			if (data->getImageIndex() == this->blank_image_index)
			{
				data->setImageIndex(this->station_images.addImage(image, this->blank_image_index));
			}
			else
			{
				if (this->station_images.replaceImage(data->getImageIndex(), image) == false)
				{
					data->setImageIndex(this->blank_image_index);
				}
			}
		}
		this->SetItemImage(item, data->getImageIndex());
	}

	return true;
}

bool
StationList::copyStation()
{
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return false;
	}

	StationList::ItemData* data = reinterpret_cast<StationList::ItemData*>(this->GetItemData(item));
	if (data == nullptr)
	{
		return false;
	}
	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[this->group_index];

	std::string json;
	if (this->editor_bookmarks->getBookmarks()->get_station_as_json(group.group, this->stations[data->getStationIndex()].name, json) == false)
	{
		return false;
	}

	// put on clipboard
	if (wxTheClipboard->Open() == false)
	{
		return false;
	}

	wxTheClipboard->SetData(new wxTextDataObject(json));
	wxTheClipboard->Close();

	return true;
}

bool
StationList::cutStation()
{
	bool status = this->copyStation();
	if (status)
	{
		status = this->deleteStation();
	}

	return status;
}

bool
StationList::isClipboardDataAvailable(bool close_when_done)
{
	if (wxTheClipboard->Open() == false)
	{
		return false;
	}

	bool status = wxTheClipboard->IsSupported(wxDF_TEXT);

	if (close_when_done)
	{
		wxTheClipboard->Close();
	}

	return status;
}

bool
StationList::pasteStation()
{
	if (this->isClipboardDataAvailable(false) == false)
	{
		wxTheClipboard->Close();
		return false;
	}

	wxTextDataObject data;
	wxTheClipboard->GetData(data);
	wxTheClipboard->Close();

	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[this->group_index];
	std::string json = data.GetText().ToStdString();
	std::string station_name;
	if (this->editor_bookmarks->getBookmarks()->add_station_from_json(group.group, json, station_name) == false)
	{
		return false;
	}
	this->editor_bookmarks->setDirty();

	// update vector
	IBookmarks::station_data_t station_data;
	if (this->editor_bookmarks->getBookmarks()->get_station(group.group, station_name, station_data) == false)
	{
		return false;
	}

	this->stations.push_back(station_data);
	int station_index = this->stations.size() - 1;
	if (station_name.compare(this->stations[station_index].name) != 0)
	{
		std::string msg = "Invalid index - " + station_name + " != " + this->stations[station_index].name + " (" + std::to_string(station_index);
		wxMessageBox(msg, wxT("Error"));
	}

	int image_index = this->station_images.addImage(station_data.image, this->blank_image_index);

	// add item
	wxString tmpstr(station_name.c_str(), wxConvUTF8);
	long item_id = this->InsertItem(this->GetItemCount(), tmpstr, image_index);
	this->SetItem(item_id, URL_COLUMN_INDEX, station_data.url);
	this->setNotify(item_id, station_data.notifications);
	this->SetItemPtrData(item_id, reinterpret_cast<wxUIntPtr>(new StationList::ItemData(item_id, this->group_index, station_index, image_index)));
	this->SetItemState(item_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	return true;
}

bool
StationList::deleteStation()
{
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return false;
	}

	StationList::ItemData* data = reinterpret_cast<StationList::ItemData*>(this->GetItemData(item));
	if (data)
	{
		wxString tmpstr(this->stations[data->getStationIndex()].name.c_str(), wxConvUTF8);
		wxString msg("Delete \"" + tmpstr + "\"\nAre you sure?");
		int status = wxMessageBox(wxString(msg), wxT("Confirm"), wxYES_NO);
		if (status == wxYES)
		{
			IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[this->group_index];
			if (this->editor_bookmarks->getBookmarks()->remove_station(group.group, this->stations[data->getStationIndex()].name) == false)
			{
				wxMessageBox(wxT("Failed to remove the station, reload the bookmarks."), wxT("Warning"));
				return false;
			}

			this->editor_bookmarks->setDirty();
			this->stations.erase(this->stations.begin() + data->getStationIndex());

			delete data;

			this->DeleteItem(item);
		}
	}

	return true;
}

void
StationList::onBeginDrag(wxListEvent& event)
{
	long item_id = event.GetIndex();
	StationList::ItemData* data = reinterpret_cast<StationList::ItemData*>(this->GetItemData(item_id));
	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[this->group_index];

	this->drag_item_id = item_id;
	this->last_selected_item_id = 0;

	std::string text;
	if (StationDragAndDrop::buildText(group.group, this->stations[data->getStationIndex()].name, item_id, text) == false)
	{
		wxMessageBox(wxT("Failed to build text object"), wxT("Error"));
		event.Veto();
		return;
	}

	this->SetDropTarget(new StationDropTarget(this));

	wxString tmpstr(text.c_str(), wxConvUTF8);
	wxTextDataObject drag_data(tmpstr);

	wxVisualAttributes attrs = this->GetDefaultAttributes();
	wxBitmap drag_image = this->makeDragImage(this->stations[data->getStationIndex()].name, &attrs.colBg, &attrs.colFg);
	wxIcon icon;
	icon.CopyFromBitmap(drag_image);
	wxDropSource drag_source(this, icon, icon, icon);
	drag_source.SetData(drag_data);
	drag_source.DoDragDrop(true);
}

// highlight station cursor is over, but do not select
bool
StationList::onStationDragOver(wxCoord x, wxCoord y)
{
	// first, let's "select" the item we are over for visual effect
	int hit_test_flags = 0;
	long hit_item_id = this->HitTest(wxPoint(x, y), hit_test_flags);

	if (this->last_selected_item_id && this->last_selected_item_id != hit_item_id)
	{
         this->SetItemState(this->last_selected_item_id, 0, wxLIST_STATE_SELECTED);
	}
	this->last_selected_item_id = 0;

	if (hit_item_id != wxNOT_FOUND && hit_item_id != this->drag_item_id)
	{
		this->SetItemState(hit_item_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		this->last_selected_item_id = hit_item_id;
	}

	// now, check to see if we need to scroll the list
	wxRect item_rect;
	long top_item_id = this->GetTopItem();
	if (this->GetItemRect(top_item_id, item_rect) == false)
	{
		return true;
	}
	int scroll_position = item_rect.GetHeight() / SCROLL_POSITION_DIVISOR;

	if (y < scroll_position)
	{
		if (top_item_id)
		{
			this->EnsureVisible(top_item_id - 1);
		}

		return true;
	}

	/// @note This is very kludgy!
	///       There is no easy, reliable way to get the bounding
	///       rectangle of the wxListCtrl object. So, we are
	///       calculating the bottom so we have a boundary to use
	///       for scrolling down the list when dragging.
	int count = this->GetCountPerPage();
	int bottom = (count * item_rect.GetHeight());

	if (y > (bottom - scroll_position))
	{
		if (hit_item_id != wxNOT_FOUND && (hit_item_id + 1) < this->GetItemCount())
		{
			this->EnsureVisible(hit_item_id + 1);
		}

		return true;
	}

	return true;
}

// move station up/down list
bool
StationList::onStationDrop(wxCoord x, wxCoord y, const wxString& data)
{
	int hit_test_flags = 0;
	long item_id = this->HitTest(wxPoint(x, y), hit_test_flags);
	if (item_id != wxNOT_FOUND)
	{
		std::string tmp_data = std::string(data.mb_str(wxConvUTF8));

		/// @todo add notifications value to drop object and handle accordingly
		std::string group;
		std::string station;
		long original_item_id;
		if (this->extractText(tmp_data, group, station, original_item_id) == false)
		{
			return false;
		}

		StationList::ItemData* data = reinterpret_cast<StationList::ItemData*>(this->GetItemData(item_id));
		if (!data)
		{
			return false;
		}

		if (station.compare(this->stations[data->getStationIndex()].name) == 0)
		{
			// same station, so no change
			return false;
		}

		if (this->editor_bookmarks->getBookmarks()->move_station_to_pos(group, station, data->getStationIndex()) == true)
		{
			this->editor_bookmarks->setDirty();
			this->loadStations(this->group_index, station);
		}
	}

	return true;
}

void
StationList::onItemRightClick(wxListEvent& event)
{
	wxMenu menu;

	if (this->editor_bookmarks.get())
	{
		menu.Append(EditorFrame::idMenuAddStation, wxT("&Add"));

		bool paste_available = this->isClipboardDataAvailable();

		long item = event.GetItem().GetId();
		if (item != -1)
		{
			menu.SetTitle(this->GetItemText(item, NAME_COLUMN_INDEX));

			menu.Append(EditorFrame::idMenuEditStation, wxT("&Edit"));
			menu.Append(EditorFrame::idMenuCopyStation, wxT("&Copy"));
			menu.Append(EditorFrame::idMenuCutStation, wxT("Cu&t"));
			if (paste_available)
			{
				menu.Append(EditorFrame::idMenuPasteStation, wxT("&Paste"));
			}
			menu.Append(EditorFrame::idMenuDeleteStation, wxT("&Delete"));
		}
		else
		{
			if (paste_available)
			{
				menu.Append(EditorFrame::idMenuPasteStation, wxT("&Paste"));
			}
		}

		menu.AppendSeparator();
	}

	menu.Append(EditorFrame::idMenuAbout, wxT("About"));

	this->PopupMenu(&menu);
}

void
StationList::setNotify(long item_id, bool checked)
{
	wxString text((checked ? wxT("Yes") : wxT("No")));
	this->SetItem(item_id, NOTIFICATION_COLUMN_INDEX, text);
}
