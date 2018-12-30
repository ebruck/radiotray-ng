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

#include <radiotray-ng/playlist/asx_decoder.hpp>
#include <gtest/gtest.h>


// test data
namespace
{
	const std::string content_type = "video/x-ms-asf";

	const std::string content =
		"<ASX version=\"3.0\">\n"
		"\n"
		"<repeat>\n"
		"\n"
		"<entry>\n"
		"<PARAM name=\"HTMLView\" value=\"http://www.977music.com/channels2/classicrock/wmplayer.php\"/>\n"
		"<ref href= \"http://7669.live.streamtheworld.com:80/977_CLASSROCK_SC\"/>\n"
		"<Abstract>Loading...977music.com</abstract>\n"
		"<Copyright>Club 977, Inc.</Copyright>\n"
		"<Author>.977 The Classic Rock Channel</author>\n"
		"<Title>.977 The Classic Rock Channel</title>\n"
		"</entry>\n"
		"\n"
		"<entry>\n"
		"<PARAM name=\"HTMLView\" value=\"http://www.977music.com/channels2/classicrock/wmplayer.php\"/>\n"
		"<ref href= \"http://7639.live.streamtheworld.com:80/977_CLASSROCK_SC\"/>\n"
		"<Abstract>Loading...977music.com</abstract>\n"
		"<Copyright>Club 977, Inc.</Copyright>\n"
		"<Author>.977 The Classic Rock Channel</author>\n"
		"<Title>.977 The Classic Rock Channel</title>\n"
		"</entry>\n"
		"\n"
		"<entry>\n"
		"<PARAM name=\"HTMLView\" value=\"http://www.977music.com/channels2/classicrock/wmplayer.php\"/>\n"
		"<ref href= \"http://7599.live.streamtheworld.com:80/977_CLASSROCK_SC\"/>\n"
		"<Abstract>Loading...977music.com</abstract>\n"
		"<Copyright>Club 977, Inc.</Copyright>\n"
		"<Author>.977 The Classic Rock Channel</author>\n"
		"<Title>.977 The Classic Rock Channel</title>\n"
		"</entry>\n"
		"\n"
		"<entry>\n"
		"<PARAM name=\"HTMLView\" value=\"http://www.977music.com/channels2/classicrock/wmplayer.php\"/>\n"
		"<ref href= \"http://7649.live.streamtheworld.com:80/977_CLASSROCK_SC\"/>\n"
		"<Abstract>Loading...977music.com</abstract>\n"
		"<Copyright>Club 977, Inc.</Copyright>\n"
		"<Author>.977 The Classic Rock Channel</author>\n"
		"<Title>.977 The Classic Rock Channel</title>\n"
		"</entry>\n"
		"\n"
		"<entry>\n"
		"<PARAM name=\"HTMLView\" value=\"http://www.977music.com/channels2/classicrock/wmplayer.php\"/>\n"
		"<ref href= \"http://7579.live.streamtheworld.com:80/977_CLASSROCK_SC\"/>\n"
		"<Abstract>Loading...977music.com</abstract>\n"
		"<Copyright>Club 977, Inc.</Copyright>\n"
		"<Author>.977 The Classic Rock Channel</author>\n"
		"<Title>.977 The Classic Rock Channel</title>\n"
		"</entry>\n"
		"\n"
		"</repeat>\n"
		"\n"
		"</ASX>\n";

	const std::string content_2 =
		"<ASX version = \"3.0\">\n"
		"<TITLE>Simple ASX Demo</TITLE>\n"
		"	<ENTRY>\n"
		"	<TITLE>An Entry in a Simple ASX</TITLE>\n"
		"	<AUTHOR>Microsoft Corporation</AUTHOR>\n"
		"	<COPYRIGHT>(c)2003 Microsoft Corporation</COPYRIGHT>\n"
		"	<!-- This is a comment.  Change the following path to point to your ASF -->\n"
		"		<REF HREF = \"mms://windowsmediaserver/path/yourfile.asf\" />\n"
		"	</ENTRY>\n"
		"</ASX>\n";
}


TEST(AsxDecoder, test_that_decoder_identifies_content_type)
{
	AsxDecoder asx_decoder;

	ASSERT_TRUE(asx_decoder.is_decodable(content_type, content));
}


TEST(AsxDecoder, test_that_playlist_can_be_decoded)
{
	AsxDecoder asx_decoder;

	playlist_t pls;
	ASSERT_FALSE(asx_decoder.decode(content_type,"", pls));
	ASSERT_TRUE(asx_decoder.decode(content_type,content, pls));
	EXPECT_EQ(pls.size(), size_t(5));
	EXPECT_EQ(pls[0], "http://7669.live.streamtheworld.com:80/977_CLASSROCK_SC");

	ASSERT_TRUE(asx_decoder.decode(content_type,content_2, pls));
}
