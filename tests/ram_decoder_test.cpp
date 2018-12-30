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

#include <radiotray-ng/playlist/ram_decoder.hpp>
#include <gtest/gtest.h>


// test data
namespace
{
	const std::string content_type = "audio/x-pn-realaudio";

	const std::string content =
		"# This is a real audio streaming resource served by a proprietary real-time protocol\n"
		"rtsp://ra2.panix.com/tutorial/sample.ra\n"
		"\n"
		"# This is a static resource\n"
		"http://www.panix.com/web/faq/multimedia/sample.ra\n"
		"\n"
		"# This opens a SMIL resource in full-screen mode:\n"
		"rtsp://realserver.example.com/media/sample1.smil?screensize=\"full\"\n"
		"\n"
		"# This opens a file on the client.\n"
		"file:///Users/lgonze/Music/mp3/misc/wtf.mp3\n";
}


TEST(RamDecoder, test_name)
{
	RamDecoder ram_decoder;

	EXPECT_EQ(ram_decoder.get_name(), "RAM");
}


TEST(RamDecoder, test_that_decoder_identifies_content_type)
{
	RamDecoder ram_decoder;

	ASSERT_TRUE(ram_decoder.is_decodable(content_type, content));
	ASSERT_FALSE(ram_decoder.is_decodable("", content));
	ASSERT_TRUE(ram_decoder.is_decodable("audio/x-pn-realaudio", ""));
	ASSERT_TRUE(ram_decoder.is_decodable("audio/vnd.rn-realaudio", ""));
	ASSERT_FALSE(ram_decoder.is_decodable("text/html", ""));
}


TEST(RamDecoder, test_that_playlist_can_be_decoded)
{
	RamDecoder pls_decoder;

	playlist_t pls;
	ASSERT_FALSE(pls_decoder.decode(content_type,"", pls));
	ASSERT_TRUE(pls_decoder.decode(content_type,content, pls));
	EXPECT_EQ(pls.size(), size_t(4));
	EXPECT_EQ(pls[0], "rtsp://ra2.panix.com/tutorial/sample.ra");
	EXPECT_EQ(pls[2], "rtsp://realserver.example.com/media/sample1.smil?screensize=\"full\"");
}
