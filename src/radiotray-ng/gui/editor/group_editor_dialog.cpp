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


IMPLEMENT_DYNAMIC_CLASS(GroupEditorDialog, wxDialog)

namespace
{
	const wxWindowID GROUP_DIALOG_ID = 401;
	const wxString GROUP_DIALOG_TITLE = wxT("Group Editor");

	const wxWindowID NAME_ID = 402;
	const wxWindowID BITMAP_ID = 403;
	const wxWindowID IMAGE_ID = 404;
	const wxWindowID IMAGE_BUTTON_ID = 405;
};

BEGIN_EVENT_TABLE(GroupEditorDialog, wxDialog)
	EVT_COMMAND(IMAGE_BUTTON_ID, wxEVT_BUTTON, GroupEditorDialog::onBrowseButton)
END_EVENT_TABLE()


GroupEditorDialog::GroupEditorDialog() :
	name_control(nullptr),
	bitmap_control(nullptr),
	image_control(nullptr),
	image_button(nullptr)
{
}

GroupEditorDialog::GroupEditorDialog(wxWindow* parent) :
	name_control(nullptr),
	bitmap_control(nullptr),
	image_control(nullptr),
	image_button(nullptr)
{
	this->create(parent, GROUP_DIALOG_ID, GROUP_DIALOG_TITLE);
}

bool
GroupEditorDialog::create(wxWindow* parent, wxWindowID id, const wxString& title)
{
    wxDialog::Create(parent, id, title);

    this->createControls();
    this->Center();

    return true;
}

bool
GroupEditorDialog::createControls()
{
	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(2, 5, 5);

	grid_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Name")), 0, wxALIGN_LEFT);
	this->name_control = new wxTextCtrl(this, NAME_ID, "", wxDefaultPosition, wxSize(140, -1));
	grid_sizer->Add(this->name_control, 0, wxALIGN_LEFT);

	grid_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Image")), 0, wxALIGN_LEFT);

	wxFlexGridSizer* image_sizer = new wxFlexGridSizer(3, 5, 5);

	wxImage image = wxImage(folder_xpm).Scale(24, 24);
	this->bitmap_control = new wxStaticBitmap(this, BITMAP_ID, wxBitmap(image));
	image_sizer->Add(this->bitmap_control, 0, wxALIGN_LEFT);
	this->image_control = new wxTextCtrl(this, IMAGE_ID, "", wxDefaultPosition, wxSize(180, -1));
	image_sizer->Add(this->image_control, 0, wxALIGN_LEFT);
	this->image_button = new wxButton(this, IMAGE_BUTTON_ID, "Browse ...");
	image_sizer->Add(this->image_button, 0, wxALIGN_RIGHT);

	grid_sizer->Add(image_sizer, 0, wxALL);

	main_sizer->Add(grid_sizer, 0, wxALL, 10);

	wxSizer* button_sizer = CreateSeparatedButtonSizer(wxOK | wxCANCEL);
	if (button_sizer)
	{
		main_sizer->Add(button_sizer, 0, wxALL|wxGROW, 5);
	}

	SetSizerAndFit(main_sizer);
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

void
GroupEditorDialog::onBrowseButton(wxCommandEvent& /* event */)
{
	std::string image = this->image_control->GetValue().ToStdString();
	wxString path = wxEmptyString;
	if (image.size())
	{
		wxFileName filename(image);
		path = filename.GetFullPath();
	}
	wxFileDialog dialog(this,
						("Select station image"),
						path,
						"",
						"Image files (*.bmp;*.ico;*.xpm;*.png;*.jpg)|*.bmp;*.ico;*.xpm;*.png;*.jpg",
						wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
	{
		return;
	}

	this->image_control->SetValue(dialog.GetPath().ToStdString());
	wxImage img = wxImage(dialog.GetPath()).Scale(24, 24);
	this->bitmap_control->SetBitmap(wxBitmap(img));
}
