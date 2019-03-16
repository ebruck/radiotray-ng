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

#include <radiotray-ng/playlist/pls_decoder.hpp>
#include <gtest/gtest.h>


// test data
namespace
{
	const std::string content_type = "audio/x-scpls;charset=UTF-8";

	const std::string content =
		"[playlist]\n"
		"File1=http://4983.live.streamtheworld.com:80/KDFCFM_SC\n"
		"File2=http://4983.live.streamtheworld.com:3690/KDFCFM_SC\n"
		"File3=http://4983.live.streamtheworld.com:443/KDFCFM_SC\n"
		"File4=http://9513.live.streamtheworld.com:80/KDFCFM_SC\n"
		"File5=http://9513.live.streamtheworld.com:3690/KDFCFM_SC\n"
		"File6=http://9513.live.streamtheworld.com:443/KDFCFM_SC\n"
		"File7=http://8343.live.streamtheworld.com:80/KDFCFM_SC\n"
		"File8=http://8343.live.streamtheworld.com:3690/KDFCFM_SC\n"
		"File9=http://8343.live.streamtheworld.com:443/KDFCFM_SC\n"
		"File10=http://9313.live.streamtheworld.com:80/KDFCFM_SC\n"
		"File11=http://9313.live.streamtheworld.com:3690/KDFCFM_SC\n"
		"File12=http://9313.live.streamtheworld.com:443/KDFCFM_SC\n"
		"File13=http://7329.live.streamtheworld.com:80/KDFCFM_SC\n"
		"File14=http://7329.live.streamtheworld.com:3690/KDFCFM_SC\n"
		"File15=http://7329.live.streamtheworld.com:443/KDFCFM_SC\n"
		"File16=http://1331.live.streamtheworld.com:80/KDFCFM_SC\n"
		"File17=http://1331.live.streamtheworld.com:3690/KDFCFM_SC\n"
		"File18=http://1331.live.streamtheworld.com:443/KDFCFM_SC\n"
		"File19=http://7279.live.streamtheworld.com:80/KDFCFM_SC\n"
		"File20=http://7279.live.streamtheworld.com:3690/KDFCFM_SC\n"
		"File21=http://7279.live.streamtheworld.com:443/KDFCFM_SC\n"
		"File22=http://2943.live.streamtheworld.com:80/KDFCFM_SC\n"
		"File23=http://2943.live.streamtheworld.com:3690/KDFCFM_SC\n"
		"File24=http://2943.live.streamtheworld.com:443/KDFCFM_SC\n"
		"File25=http://2943.live.streamtheworld.com:8000/KDFCFM_SC\n"
		"File26=\n"
		"Title1=KDFCFM_SC\n"
		"Title2=KDFCFM_SC-Bak\n"
		"Length1=-1\n"
		"NumberOfEntries=25\n"
		"Version=2\n";

	const std::string content2 =
		"[playlist]\r"
		"numberofentries=3\r"
		"File1=http://sj128.hnux.com\r"
		"Title1=SmoothJazz.com Global Radio\nThe World's First 24/7 Since 2000 - KJAZ.DB\r"
		"Length1=-1\r"
		"File2=http://stream-05.shoutcast.com:8002 sid=8\r"
		"Title2=SmoothJazz.com Global Radio - The World's First 24/7 Since 2000 - KJAZ.DB\r"
		"Length2=-1\r"
		"File3=http://stream-94.shoutcast.com sid=1\r"
		"Title3=SmoothJazz.com Global Radio - The World's First 24/7 Since 2000 - KJAZ.DB\r"
		"Length3=-1\r";
}


TEST(PlsDecoder, test_that_decoder_identifies_content_type)
{
	PlsDecoder pls_decoder;

	ASSERT_TRUE(pls_decoder.is_decodable(content_type, content));
	ASSERT_TRUE(pls_decoder.is_decodable("", content));
	ASSERT_TRUE(pls_decoder.is_decodable("application/pls+xml;charset=UTF-8", ""));
	ASSERT_FALSE(pls_decoder.is_decodable("text/html", ""));
}


TEST(PlsDecoder, test_that_playlist_can_be_decoded)
{
	PlsDecoder pls_decoder;

	// \n line terminator
	{
		playlist_t pls;
		ASSERT_FALSE(pls_decoder.decode(content_type,"", pls));
		ASSERT_TRUE(pls_decoder.decode(content_type,content, pls));
		EXPECT_EQ(pls.size(), size_t(25));
		EXPECT_EQ(pls[0], "http://4983.live.streamtheworld.com:80/KDFCFM_SC");
		EXPECT_EQ(pls[24],"http://2943.live.streamtheworld.com:8000/KDFCFM_SC");
	}

	// \r line terminator
	{
		playlist_t pls;
		ASSERT_TRUE(pls_decoder.decode(content_type,content2, pls));
		EXPECT_EQ(pls.size(), size_t(3));
		EXPECT_EQ(pls[0], "http://sj128.hnux.com");
		EXPECT_EQ(pls[2],"http://stream-94.shoutcast.com sid=1");
	}
}
