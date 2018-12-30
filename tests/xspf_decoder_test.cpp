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

#include <radiotray-ng/playlist/xspf_decoder.hpp>
#include <gtest/gtest.h>


// test data
namespace
{
	const std::string content_type = "application/xspf+xml";

	const std::string content =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<playlist version=\"1\" xmlns = \"http://xspf.org/ns/0/\">\n"
		"  <trackList>\n"
		"    <track><location>http://example.com/song_1.mp3</location></track>\n"
		"    <track><location>http://example.com/song_2.mp3</location></track>\n"
		"    <track><location > http://example.com/song_3.mp3 </ location></track>\n"
		"  </trackList>\n"
		"</playlist>\n"
		"\n";
}


TEST(XspfDecoder, test_that_decoder_identifies_content_type)
{
	XspfDecoder xspf_decoder;

	ASSERT_TRUE(xspf_decoder.is_decodable(content_type, content));
}


TEST(XspfDecoder, test_that_playlist_can_be_decoded)
{
	XspfDecoder xspf_decoder;

	playlist_t pls;
	ASSERT_FALSE(xspf_decoder.decode(content_type,"", pls));
	ASSERT_TRUE(xspf_decoder.decode(content_type,content, pls));
	EXPECT_EQ(pls.size(), size_t(3));
	EXPECT_EQ(pls[0], "http://example.com/song_1.mp3");
	EXPECT_EQ(pls[2], "http://example.com/song_3.mp3");
}
