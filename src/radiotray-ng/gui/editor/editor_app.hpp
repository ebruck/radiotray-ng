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
	#include <wx/snglinst.h>
	#include <wx/cmdline.h>
	#include <memory>
#endif

#include "editor_frame.hpp"

#define	PROJECT_NAME			"Radiotray-NG"
#define	APPLICATION_NAME		"Bookmark Editor"
#define APPLICATION_PID_NAME	"rtng-bookmark-editor.pid"


namespace
{
	const std::string TEXT_DELIMITER("\t");
}


class EditorApp : public wxApp
{
public:
	EditorApp();
	virtual ~EditorApp() = default;

	virtual bool OnInit();
	virtual int OnExit();

	virtual void OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

	std::shared_ptr<wxConfig> getConfig() { return this->config; };
	const std::string& getResourcePath() const { return this->resource_path; };

private:
	EditorFrame* frame;
	std::string file_to_load;

	std::unique_ptr<wxSingleInstanceChecker> instance_checker;
	std::shared_ptr<wxConfig> config;
	std::string resource_path;
};

wxDECLARE_APP(EditorApp);
