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

#ifndef _WX_PRECOMPILED_HEADER
#define _WX_PRECOMPILED_HEADER

// basic wxWidgets headers
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#ifdef WX_PRECOMP
    // put here all your rarely-changing header files
    #include <wx/splitter.h>
	#include <wx/listctrl.h>
	#include <wx/filedlg.h>
    #include <wx/config.h>
    #include <wx/imaglist.h>
    #include <wx/filename.h>
    #include <wx/dnd.h>
	#include <wx/snglinst.h>
	#include <wx/cmdline.h>
	#include <wx/clipbrd.h>

    #include <memory>
    #include <atomic>
    #include <vector>
    #include <string>

    #include <radiotray-ng/common.hpp>
    #include <radiotray-ng/helpers.hpp>
#endif // WX_PRECOMP

#endif // _WX_PRECOMPILED_HEADER
