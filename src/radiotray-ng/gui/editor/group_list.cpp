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
    #include <wx/dnd.h>
	#include <wx/clipbrd.h>
	#include <memory>

    #include <radiotray-ng/common.hpp>
    #include <radiotray-ng/helpers.hpp>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "group_list.hpp"
#include "editor_app.hpp"
#include "group_editor_dialog.hpp"

#include "images/folder.xpm"
#include "images/move-icon.xpm"


IMPLEMENT_DYNAMIC_CLASS(GroupList, wxListCtrl)


namespace
{
	const wxString NAME_COLUMN_TEXT(wxT("Group"));
	const long NAME_COLUMN_INDEX = 0;
	const int NAME_COLUMN_WIDTH = 160;

	const int SCROLL_POSITION_DIVISOR = 3;

	const wxString CONFIG_PATH(wxT("/group"));
	const wxString CONFIG_COLUMN_1(wxT("col1"));

	const std::string ROOT_NAME("root");
};


BEGIN_EVENT_TABLE(GroupList, wxListCtrl)
	EVT_LIST_ITEM_SELECTED(GROUP_LIST_ID, GroupList::onItemSelected)
	EVT_LIST_DELETE_ALL_ITEMS(GROUP_LIST_ID, GroupList::onDeleteAllItems)
	EVT_LIST_BEGIN_DRAG(GROUP_LIST_ID, GroupList::onBeginDrag)
	EVT_LIST_ITEM_RIGHT_CLICK(GROUP_LIST_ID, GroupList::onItemRightClick)
END_EVENT_TABLE()


GroupList::GroupList() :
	folder_image_index(-1),
	have_root(false),
	last_selected_item_id(0),
	drag_item_id(0),
	station_list(nullptr)
{
	this->root_name = "[" + ROOT_NAME + "]";
}

GroupList::GroupList(wxWindow* parent, StationList* list) :
	wxListCtrl(parent, GROUP_LIST_ID),
	have_root(false),
	last_selected_item_id(0),
	drag_item_id(0),
	station_list(list)
{
	this->root_name = "[" + ROOT_NAME + "]";

	long style = wxLC_REPORT | wxLC_SINGLE_SEL;
	this->SetWindowStyle(style);

	// create headers
	this->InsertColumn(NAME_COLUMN_INDEX, NAME_COLUMN_TEXT);

	this->folder_image_index = this->group_images.addImage(wxImage(folder_xpm));
	this->SetImageList(this->group_images.getList().get(), wxIMAGE_LIST_SMALL);

	this->SetDropTarget(new StationDropTarget(this));
}

GroupList::~GroupList()
{
}

bool
GroupList::saveConfiguration()
{
	std::shared_ptr<wxConfig> config = static_cast<EditorApp*>(wxTheApp)->getConfig();

	config->SetPath(CONFIG_PATH);
	config->Write(CONFIG_COLUMN_1, this->GetColumnWidth(NAME_COLUMN_INDEX));

	return true;
}

bool
GroupList::restoreConfiguration()
{
	std::shared_ptr<wxConfig> config = static_cast<EditorApp*>(wxTheApp)->getConfig();

	config->SetPath(CONFIG_PATH);

    int col1 = config->Read(CONFIG_COLUMN_1, NAME_COLUMN_WIDTH);
    this->SetColumnWidth(0, col1);

	return true;
}

void
GroupList::onItemSelected(wxListEvent& event)
{
    long id = event.GetItem();
    GroupList::ItemData* data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(id));

    this->station_list->loadStations(data->getBookmarksIndex());
}

void
GroupList::onDeleteAllItems(wxListEvent& /* event */)
{
	this->station_list->clearStations();

	long item = -1;
	for ( ;; )
	{
		item = this->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
		if (item == -1)
		{
			break;
		}

		GroupList::ItemData* data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(item));
		if (data)
		{
			delete data;
		}
	}
}

void
GroupList::clearGroups()
{
	this->DeleteAllItems();
	this->group_images.RemoveAll();
	this->have_root = false;
}

bool
GroupList::loadBookmarks(std::shared_ptr<EditorBookmarks> bkm)
{
	if (this->editor_bookmarks.get())
	{
		this->clearGroups();
		this->editor_bookmarks.reset();
	}

	this->editor_bookmarks = bkm;
	this->station_list->setBookmarks(this->editor_bookmarks);

	if (this->editor_bookmarks->getBookmarks()->load() == false)
	{
		return false;
	}

	this->populateList();

	return true;
}

void
GroupList::populateList(size_t select_index)
{
	long select_item = -1;
	for (size_t index = 0; index < this->editor_bookmarks->getBookmarks()->size(); index++)
	{
		IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[index];

		std::string name = group.group;
		bool is_root = false;
		if (group.group.compare(ROOT_NAME) == 0)
		{
			this->have_root = true;
			name = this->root_name;
			is_root = true;
		}

		int use_image_index = this->group_images.addImage(group.image, this->folder_image_index);

		long item_index = this->GetItemCount();
		if (this->have_root && !is_root)
		{
			--item_index;
		}
		wxString tmpstr(name.c_str(), wxConvUTF8);
		long item_id = this->InsertItem(item_index, tmpstr, use_image_index);
		this->SetItemPtrData(item_id, reinterpret_cast<wxUIntPtr>(new GroupList::ItemData(index, use_image_index)));

		if (select_item == -1)
		{
			if (select_index == static_cast<size_t>(-1) || select_index == index)
			{
				select_item = item_id;
			}
		}
	}

	if (have_root == false)
	{
		if (this->editor_bookmarks->getBookmarks()->add_group(ROOT_NAME, "") == true)
		{
			this->have_root = true;

			size_t index = 0;
			for (index = 0; index < this->editor_bookmarks->getBookmarks()->size(); index++)
			{
				IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[index];
				if (group.group.compare(ROOT_NAME) == 0)
				{
					break;
				}
			}

			wxString tmpstr(this->root_name.c_str(), wxConvUTF8);
			long item_id = this->InsertItem(this->GetItemCount(), tmpstr, this->folder_image_index);
			this->SetItemPtrData(item_id, reinterpret_cast<wxUIntPtr>(new GroupList::ItemData(index, this->folder_image_index)));
		}
	}

	if (select_item != -1)
	{
		this->SetItemState(select_item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

bool
GroupList::doNew(std::shared_ptr<EditorBookmarks> bkm)
{
	this->editor_bookmarks = bkm;
	this->station_list->setBookmarks(this->editor_bookmarks);

	if (this->editor_bookmarks->getBookmarks()->add_group(ROOT_NAME, "") == true)
	{
		this->have_root = true;

		size_t index = 0;
		for (index = 0; index < this->editor_bookmarks->getBookmarks()->size(); index++)
		{
			IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[index];
			if (group.group.compare(ROOT_NAME) == 0)
			{
				break;
			}
		}

		wxString tmpstr(this->root_name.c_str(), wxConvUTF8);
		long item_id = this->InsertItem(this->GetItemCount(), tmpstr, this->folder_image_index);
		this->SetItemPtrData(item_id, reinterpret_cast<wxUIntPtr>(new GroupList::ItemData(index, this->folder_image_index)));
	}

	return true;
}

bool
GroupList::addGroup()
{
	GroupEditorDialog dlg(this->GetParent());
	if (dlg.ShowModal() != wxID_OK)
	{
		dlg.Destroy();
		return true;
	}

	std::string name, image;
	dlg.getData(name, image);
	dlg.Destroy();

	// trim data
	name = radiotray_ng::trim(name);
	image = radiotray_ng::trim(image);

	// cannot add empty or "root"
	if (name.size() == 0 || name.compare(this->root_name) == 0)
	{
		return false;
	}

	// verify image exists, clear if not
	wxFileName image_file(radiotray_ng::word_expand(image));
	if (image_file.Exists() == false || image_file.IsFileReadable() == false)
	{
		image = "";
	}

	/// @note The assumption here is that we always keep "root" at the end of the list
	size_t root_index = this->editor_bookmarks->getBookmarks()->size();
	if (this->editor_bookmarks->getBookmarks()->add_group(name, image) == false)
	{
		wxMessageBox(wxT("Failed to add the new group!"), wxT("Error"));
		return false;
	}
	this->editor_bookmarks->setDirty();

	// always make sure root is at the end
	if (this->have_root)
	{
		LOG(debug) << "moving new group to position " << root_index;
		this->editor_bookmarks->getBookmarks()->move_group_to_pos(name, root_index);
	}

	size_t index;
	bool found = false;
	for (index = 0; index < this->editor_bookmarks->getBookmarks()->size(); index++)
	{
		IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[index];
		if (name.compare(group.group) == 0)
		{
			found = true;
			break;
		}
	}
	if (found == false)
	{
		wxMessageBox(wxT("An error occurred during the addition process."), wxT("Error"));
		return false;
	}

	int use_image_index = this->group_images.addImage(image, this->folder_image_index);

	wxString tmpstr(name.c_str(), wxConvUTF8);
	long item_id = this->InsertItem(this->GetItemCount() - 1, tmpstr, use_image_index);
	this->SetItemPtrData(item_id, reinterpret_cast<wxUIntPtr>(new GroupList::ItemData(index, use_image_index)));

	this->reindexGroups();

	this->SetItemState(item_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	return true;
}

bool
GroupList::editGroup()
{
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return false;
	}

	GroupList::ItemData* data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(item));
	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[data->getBookmarksIndex()];

	// cannot edit "root"
	if (group.group.compare(ROOT_NAME) == 0)
	{
		return false;
	}

	wxString tmpstr = this->GetItemText(item, NAME_COLUMN_INDEX);
	std::string original_name = std::string(tmpstr.mb_str(wxConvUTF8));
	std::string original_image = group.image;

	GroupEditorDialog dlg(this->GetParent());
	dlg.setData(original_name, original_image);
	if (dlg.ShowModal() != wxID_OK)
	{
		dlg.Destroy();
		return true;
	}

	std::string name, image;
	dlg.getData(name, image);
	dlg.Destroy();

	// trim data
	name = radiotray_ng::trim(name);
	image = radiotray_ng::trim(image);

	if (name.compare(original_name) == 0 &&
		image.compare(original_image) == 0)
	{
		// no changes, so bail
		return true;
	}

	// verify image exists, clear if not
	if (!image.empty())
	{
		wxFileName image_file(radiotray_ng::word_expand(image));
		if (image_file.Exists() == false || image_file.IsFileReadable() == false)
		{
			image = original_image;
		}
	}

	// update data
	if (name.compare(original_name) != 0)
	{
		if (this->editor_bookmarks->getBookmarks()->rename_group(group.group, name))
		{
			this->editor_bookmarks->setDirty();
		}
	}
	if (image.compare(original_image) != 0)
	{
		if (this->editor_bookmarks->getBookmarks()->update_group(group.group, image))
		{
			this->editor_bookmarks->setDirty();
		}
	}

	// update list data
	tmpstr = wxString(name.c_str(), wxConvUTF8);
	this->SetItem(item, 0, tmpstr);

	if (image.compare(original_image) != 0)
	{
		if (image.size() == 0)
		{
			data->setImageIndex(this->folder_image_index);
		}
		else
		{
			if (data->getImageIndex() == this->folder_image_index)
			{
				data->setImageIndex(this->group_images.addImage(image, this->folder_image_index));
			}
			else
			{
				this->group_images.replaceImage(data->getImageIndex(), image);
			}
		}
		this->SetItemImage(item, data->getImageIndex());
	}

	return true;
}

bool
GroupList::copyGroup()
{
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return false;
	}

    GroupList::ItemData* data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(item));
	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[data->getBookmarksIndex()];

	std::string json;
	if (this->editor_bookmarks->getBookmarks()->get_group_as_json(group.group, json) == false)
	{
		return false;
	}

	// put on clipboard
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(json));
		wxTheClipboard->Close();
	}

	return true;
}

bool
GroupList::deleteGroup()
{
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return false;
	}

    GroupList::ItemData* data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(item));
	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[data->getBookmarksIndex()];

	// cannot delete "root"
	if (group.group.compare(ROOT_NAME) == 0)
	{
		return false;
	}

	wxString tmpstr(group.group.c_str(), wxConvUTF8);
	wxString msg("Delete \"" + tmpstr + "\"\nAre you sure?");
	int status = wxMessageBox(wxString(msg), wxT("Confirm"), wxYES_NO);
	if (status == wxYES)
	{
		this->station_list->clearStations();

		if (this->editor_bookmarks->getBookmarks()->remove_group(group.group) == false)
		{
			wxMessageBox(wxT("Failed to remove the group."), wxT("Error"));
		}
		else
		{
			this->editor_bookmarks->setDirty();

			delete data;
			this->DeleteItem(item);

			this->reindexGroups();
		}
	}

	return true;
}

void
GroupList::reindexGroups()
{
	for (size_t index = 0; index < this->editor_bookmarks->getBookmarks()->size(); index++)
	{
		IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[index];
		long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
		while (item != -1)
		{
			std::string value = this->GetItemText(item, 0).ToStdString();
			if (value.compare(group.group) == 0)
			{
				GroupList::ItemData* data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(item));
				data->setBookmarksIndex(index);
				break;
			}

		item = this->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
		}
	}
}

void
GroupList::onBeginDrag(wxListEvent& event)
{
	long item_id = event.GetIndex();
	GroupList::ItemData* data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(item_id));
	IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[data->getBookmarksIndex()];

	// cannot drag "root"
	if (group.group.compare(ROOT_NAME) == 0)
	{
		event.Veto();
		return;
	}

	std::string text;
	if (GroupDragAndDrop::buildText(group.group, item_id, text) == false)
	{
		wxMessageBox(wxT("Failed to build text object"), wxT("Error"));
		event.Veto();
		return;
	}

	this->drag_item_id = item_id;
	this->last_selected_item_id = 0;

	this->SetDropTarget(new GroupDropTarget(this));

	wxString tmpstr(text.c_str(), wxConvUTF8);
	wxTextDataObject drag_data(tmpstr);

	wxVisualAttributes attrs = this->GetDefaultAttributes();
	wxBitmap drag_image = this->makeDragImage(group.group, &attrs.colBg, &attrs.colFg);
	wxIcon icon;
	icon.CopyFromBitmap(drag_image);
	wxDropSource drag_source(this, icon, icon, icon);
	drag_source.SetData(drag_data);
	drag_source.DoDragDrop(true);

	this->SetDropTarget(new StationDropTarget(this));
}

// highlight group cursor is over, but do no select
bool
GroupList::onGroupDragOver(wxCoord x, wxCoord y)
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

// move group to up/down list
bool
GroupList::onGroupDrop(wxCoord x, wxCoord y, const wxString& data)
{
	int hit_test_flags = 0;
	long item_id = this->HitTest(wxPoint(x, y), hit_test_flags);
	if (item_id != wxNOT_FOUND)
	{
		std::string tmp_data = std::string(data.mb_str(wxConvUTF8));

		std::string group;
		long original_item_id;
		if (GroupDragAndDrop::extractText(tmp_data, group, original_item_id) == false)
		{
			return false;
		}

		GroupList::ItemData* group_data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(item_id));
		if (!group_data)
		{
			return false;
		}

		IBookmarks::group_data_t group_drop = (*this->editor_bookmarks->getBookmarks().get())[group_data->getBookmarksIndex()];
		if (group.compare(group_drop.group) == 0)
		{
			// same group, no change
			return false;
		}

		size_t target_index = group_data->getBookmarksIndex();
		if (group_drop.group.compare(ROOT_NAME) == 0)
		{
			--target_index;
		}
		if (this->editor_bookmarks->getBookmarks()->move_group_to_pos(group, target_index) == true)
		{
			this->editor_bookmarks->setDirty();

			this->clearGroups();
			this->populateList(target_index);
		}
	}

	return true;
}

// highlight group cursor is over, but do not select
bool
GroupList::onStationDragOver(wxCoord x, wxCoord y)
{
	// first, let's "select" the item we are over for visual effect
	int hit_test_flags = 0;
	long hit_item_id = this->HitTest(wxPoint(x, y), hit_test_flags);

	if (this->last_selected_item_id && this->last_selected_item_id != hit_item_id)
	{
         this->SetItemState(this->last_selected_item_id, 0, wxLIST_STATE_SELECTED);
	}
	this->last_selected_item_id = 0;

	if (hit_item_id != wxNOT_FOUND)
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

// move station from one group to another
bool
GroupList::onStationDrop(wxCoord x, wxCoord y, const wxString& data)
{
	int hit_test_flags = 0;
	long item_id = this->HitTest(wxPoint(x, y), hit_test_flags);
	if (item_id != wxNOT_FOUND)
	{
		std::string tmp_data = std::string(data.mb_str(wxConvUTF8));

		std::string original_group;
		std::string station;
		long original_item_id;
		if (StationDragAndDrop::extractText(tmp_data, original_group, station, original_item_id) == false)
		{
			return false;
		}

		GroupList::ItemData* item_data = reinterpret_cast<GroupList::ItemData*>(this->GetItemData(item_id));
		if (!item_data)
		{
			return false;
		}

		IBookmarks::group_data_t group = (*this->editor_bookmarks->getBookmarks().get())[item_data->getBookmarksIndex()];

		if (original_group.compare(group.group) == 0)
		{
			// same group, so no change
			return false;
		}

		IBookmarks::station_data_t station_data;
		if (this->editor_bookmarks->getBookmarks()->get_station(original_group, station, station_data) == false)
		{
			return false;
		}

		if (this->editor_bookmarks->getBookmarks()->add_station(group.group, station_data.name, station_data.url, station_data.image, station_data.notifications) == false)
		{
			return false;
		}
		this->editor_bookmarks->getBookmarks()->remove_station(original_group, station);
		this->editor_bookmarks->setDirty();

		this->SetItemState(item_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		this->station_list->loadStations(item_data->getBookmarksIndex(), station);
	}

	return true;
}

void
GroupList::onItemRightClick(wxListEvent& event)
{
	wxMenu menu;

	if (this->editor_bookmarks.get())
	{
		menu.Append(EditorFrame::idMenuAddGroup, wxT("&Add"));

		long item_id = event.GetItem().GetId();
		if (item_id != -1)
		{
			menu.SetTitle(this->GetItemText(item_id, NAME_COLUMN_INDEX));

			menu.Append(EditorFrame::idMenuEditGroup, wxT("&Edit"));
			menu.Append(EditorFrame::idMenuCopyGroup, wxT("&Copy"));
			menu.Append(EditorFrame::idMenuDeleteGroup, wxT("&Delete"));
		}

		menu.AppendSeparator();
	}

	menu.Append(EditorFrame::idMenuAbout, wxT("About"));

	this->PopupMenu(&menu);
}
