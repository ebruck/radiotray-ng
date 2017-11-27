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
	#include <wx/filedlg.h>
	#include <wx/filename.h>

	#include <radiotray-ng/helpers.hpp>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "group_editor_dialog.hpp"

#include "images/folder.xpm"


//IMPLEMENT_DYNAMIC_CLASS(GroupEditorDialog, EditorDialogBase)

namespace
{
	const wxWindowID GROUP_DIALOG_ID = 401;
	const wxString GROUP_DIALOG_TITLE = wxT("Group Editor");

	const wxWindowID NAME_ID = 402;
	const wxWindowID BITMAP_ID = 403;
	const wxWindowID IMAGE_ID = 404;
};

BEGIN_EVENT_TABLE(GroupEditorDialog, EditorDialogBase)
END_EVENT_TABLE()


GroupEditorDialog::GroupEditorDialog() : EditorDialogBase(),
	name_control(nullptr),
	bitmap_control(nullptr),
	image_control(nullptr)
{
}

GroupEditorDialog::GroupEditorDialog(wxWindow* parent) : GroupEditorDialog()
{
	this->create(parent, GROUP_DIALOG_ID, GROUP_DIALOG_TITLE);
}

GroupEditorDialog::~GroupEditorDialog()
{
	if (image_control)
	{
		delete image_control;
	}

	if (bitmap_control)
	{
		delete bitmap_control;
	}

	if (name_control)
	{
		delete name_control;
	}
}

bool
GroupEditorDialog::createControls()
{
	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(2, 5, 5);

	grid_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Name")), 0, wxALIGN_LEFT);
	this->name_control = new wxTextCtrl(this, NAME_ID, "", wxDefaultPosition, wxSize(140, -1));
	grid_sizer->Add(this->name_control, 0, wxALIGN_LEFT | wxEXPAND);

	grid_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Image")), 0, wxALIGN_LEFT);

	wxFlexGridSizer* image_sizer = new wxFlexGridSizer(3, 5, 5);

	wxImage image = wxImage(folder_xpm).Scale(24, 24);
	this->bitmap_control = new wxStaticBitmap(this, BITMAP_ID, wxBitmap(image));
	image_sizer->Add(this->bitmap_control, 0, wxALIGN_LEFT);
	this->image_control = new wxTextCtrl(this, IMAGE_ID, "", wxDefaultPosition, wxSize(270, -1));
	image_sizer->Add(this->image_control, 0, wxALIGN_LEFT | wxEXPAND);

	this->addImageButton(image_sizer);

	grid_sizer->Add(image_sizer, 0, wxALL);

	main_sizer->Add(grid_sizer, 0, wxALL, 10);

	this->finishDialog(main_sizer);

	this->name_control->SetFocus();

	return true;
}

void
GroupEditorDialog::setData(const std::string& name, const std::string& image)
{
	wxString tmpstr(name.c_str(), wxConvUTF8);
	this->name_control->SetValue(tmpstr);
	this->image_control->SetValue(wxString(image));

	if (!image.empty())
	{
		/// @todo The following is a "temporary" workaround for the
		///       issue identified in 15331. This can be removed once
		///       wxWidgets 3.1 is available.
		///
		///       http://trac.wxwidgets.org/ticket/15331
		wxLogNull log_null;

		wxImage img = wxImage(radiotray_ng::word_expand(image)).Scale(24, 24);
		this->bitmap_control->SetBitmap(wxBitmap(img));
	}
}

void
GroupEditorDialog::getData(std::string& name, std::string& image)
{
	wxString tmpstr = this->name_control->GetValue();
	name = std::string(tmpstr.mb_str(wxConvUTF8));
	name = radiotray_ng::trim(name);
	image = this->image_control->GetValue().ToStdString();
}

std::string
GroupEditorDialog::getImagePath()
{
	return this->image_control->GetValue().ToStdString();
}

bool
GroupEditorDialog::setImage(const std::string& path)
{
	this->image_control->SetValue(path);
	wxImage img = wxImage(path).Scale(24, 24);
	this->bitmap_control->SetBitmap(wxBitmap(img));

	return true;
}
