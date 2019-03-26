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
    #include <wx/config.h>
	#include <memory>

	#include <radiotray-ng/common.hpp>
	#include <radiotray-ng/helpers.hpp>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "editor_app.hpp"
#include "editor_frame.hpp"

wxIMPLEMENT_APP(EditorApp);


EditorApp::EditorApp() :
	frame(nullptr),
	resource_path(RTNG_DEFAULT_INSTALL_DIR)
{
}

void
EditorApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	wxApp::OnInitCmdLine(parser);
	parser.AddParam(_("Bookmark file to load"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
}

bool
EditorApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	LOG(debug) << "onCmdLineParsed: " << parser.GetParamCount();

	if (parser.GetParamCount())
	{
		this->file_to_load = parser.GetParam();
		LOG(debug) << " -- file to load = " << this->file_to_load;
	}

	return wxApp::OnCmdLineParsed(parser);
}

bool
EditorApp::OnInit()
{
	this->instance_checker = std::make_unique<wxSingleInstanceChecker>(APPLICATION_PID_NAME, radiotray_ng::get_runtime_dir());
	if (this->instance_checker->IsAnotherRunning())
	{
		wxLogError(_("Another instance is already running, aborting."));
		return false;
	}

	wxInitAllImageHandlers();

	if (!wxApp::OnInit())
	{
		return false;
	}

	std::string local_file = radiotray_ng::get_data_dir(APP_NAME);
	local_file += wxApp::GetAppName().ToStdString();
	local_file += ".cfg";

	this->config = std::make_unique<wxConfig>(wxApp::GetAppName(),
										APP_NAME,
										local_file,
										wxEmptyString,
										wxCONFIG_USE_LOCAL_FILE);
	this->config->SetRecordDefaults();

	std::string title = PROJECT_NAME " " APPLICATION_NAME;

    this->frame = new EditorFrame(nullptr, title, this->file_to_load);

    std::string image_name(this->getResourcePath() + RADIOTRAY_NG_LOGO_ICON);
    wxBitmap bitmap(image_name, wxBITMAP_TYPE_PNG);
	wxIcon icon;
	icon.CopyFromBitmap(bitmap);
    this->frame->SetIcon(icon);

    this->frame->Show();

    return true;
}

int
EditorApp::OnExit()
{
	return 0;
}

