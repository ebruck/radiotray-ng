// Copyright 2017 Edward G. Bruck <ed.bruck1@gmail.com>
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

#include <radiotray-ng/playlist/asf_decoder.hpp>
#include <gtest/gtest.h>


// test data
namespace
{
	const std::string content_type = "video/x-ms-asf";

	// ./playlist_runner http://johngreek.live24.gr/polis2795
	const std::string content =
		"[Reference]\n"
		"Ref1=mms://windowsmediaserver/path/yourfile.asf\n"
		"Ref2=http://johngreek.live24.gr/polis2795?MSWMExt=.asf\n"
		"\n";
	}


TEST(AsfDecoder, test_that_decoder_identifies_content_type)
{
	AsfDecoder asf_decoder;

	ASSERT_TRUE(asf_decoder.is_decodable(content_type, content));
}


TEST(AsfDecoder, test_that_playlist_can_be_decoded)
{
	AsfDecoder asf_decoder;

	playlist_t pls;
	ASSERT_FALSE(asf_decoder.decode(content_type,"", pls));
	ASSERT_TRUE(asf_decoder.decode(content_type,content, pls));
	EXPECT_EQ(pls.size(), size_t(2));
	EXPECT_EQ(pls[0], "mms://windowsmediaserver/path/yourfile.asf");
	EXPECT_EQ(pls[1], "mms://johngreek.live24.gr/polis2795?MSWMExt=.asf");
}

