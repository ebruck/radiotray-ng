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

#include "editor_dialog_base.hpp"


namespace
{
	const wxWindowID IMAGE_BUTTON_ID = 375;
};

BEGIN_EVENT_TABLE(EditorDialogBase, wxDialog)
	EVT_COMMAND(IMAGE_BUTTON_ID, wxEVT_BUTTON, EditorDialogBase::onBrowseButton)
END_EVENT_TABLE()


EditorDialogBase::EditorDialogBase() :
	image_button(nullptr)
{
}

EditorDialogBase::~EditorDialogBase()
{
	if (image_button)
	{
		delete image_button;
	}
}

bool
EditorDialogBase::create(wxWindow* parent, wxWindowID id, const wxString& title)
{
    wxDialog::Create(parent, id, title);

    this->createControls();
    this->Center();
    this->SetWindowStyle(wxCAPTION);

    return true;
}

bool
EditorDialogBase::addImageButton(wxSizer* image_sizer)
{
	if (image_sizer == nullptr)
	{
		return false;
	}

	this->image_button = new wxButton(this, IMAGE_BUTTON_ID, "Browse ...");
	image_sizer->Add(this->image_button, 0, wxALIGN_RIGHT);

	return true;
}

bool
EditorDialogBase::finishDialog(wxSizer* main_sizer)
{
	if (main_sizer == nullptr)
	{
		return false;
	}

	wxSizer* button_sizer = CreateSeparatedButtonSizer(wxOK | wxCANCEL);
	if (button_sizer)
	{
		main_sizer->Add(button_sizer, 0, wxALL|wxGROW, 5);
	}

	SetSizerAndFit(main_sizer);

	return true;
}

void
EditorDialogBase::onBrowseButton(wxCommandEvent& /* event */)
{
	std::string image = this->getImagePath();
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

	this->setImage(dialog.GetPath().ToStdString());
}

