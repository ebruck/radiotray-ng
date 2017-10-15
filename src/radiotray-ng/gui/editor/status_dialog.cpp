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
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "status_dialog.hpp"


StatusDialog::StatusDialog()
{
	this->Create(nullptr, wxID_ANY, wxT("Status Dialog"));

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	for (int i = 0; i < MAX_TEXT_FIELDS; i++)
	{
		this->text_fields[i] = new wxStaticText(this, wxID_ANY, wxT("nothing"), wxDefaultPosition, wxSize(240, -1));
		main_sizer->Add(this->text_fields[i], wxEXPAND | wxALL);
	}

	this->SetSizerAndFit(main_sizer);
}

StatusDialog::~StatusDialog()
{
}

void
StatusDialog::setStatus(int field, const std::string& message)
{
	if (field >= 0 && field < MAX_TEXT_FIELDS)
	{
		this->text_fields[field]->SetLabel(message);
	}
}
