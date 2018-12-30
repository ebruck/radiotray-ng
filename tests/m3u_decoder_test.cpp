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

#include <radiotray-ng/playlist/m3u_decoder.hpp>
#include <gtest/gtest.h>


// test data
namespace
{
	const std::string content_type = "audio/x-mpegurl";

	const std::string content_type_uri_list1 = "text/html";
	const std::string content_type_uri_list2 = "text/uri-list";

	// curl -I  http://kuscstream.org/mp3/kusc128.m3u
	const std::string headers =
		"HTTP/1.1 200 OK"
		"Date: Wed, 03 Dec 2014 21:24:01 GMT"
		"Server: Apache/2.2.22"
		"Last-Modified: Wed, 03 Jul 2013 04:35:42 GMT"
		"ETag: \"472a295-2d-4e093fe8be65b\""
		"Accept-Ranges: bytes"
		"Content-Length: 45"
		"Vary: User-Agent"
		"Content-Type: audio/x-mpegurl";

	const std::string playlist =
		"# comment\n"
		"http://kusc-ice.streamguys.com:80/kusc-128k\n"
		"http://kusc-ice.streamguys.com:8080/kusc-128k\n"
		"#http://kusc-ice.streamguys.com:80/kusc-128k\n";

	const std::string bad_uri_playlist =
		"# comment\n"
		"ftp://kusc-ice.streamguys.com:80/kusc-128k\n";
}


TEST(M3uDecoder, test_that_decoder_identifies_content_type)
{
	M3uDecoder m3u_decoder;

	ASSERT_TRUE(m3u_decoder.is_decodable(headers, playlist));
	ASSERT_TRUE(m3u_decoder.is_decodable("audio/mpegurl;charset=UTF-8", ""));
	ASSERT_TRUE(m3u_decoder.is_decodable("audio/x-mpegurl", ""));
	ASSERT_TRUE(m3u_decoder.is_decodable(content_type_uri_list1, ""));
	ASSERT_TRUE(m3u_decoder.is_decodable(content_type_uri_list2, ""));
}


TEST(M3uDecoder, test_that_playlist_can_be_decoded)
{
	M3uDecoder m3u_decoder;

	playlist_t pls;
	ASSERT_FALSE(m3u_decoder.decode(content_type,"", pls));
	ASSERT_TRUE(m3u_decoder.decode(content_type,playlist, pls));
	EXPECT_EQ(pls.size(), size_t(2));
	EXPECT_EQ(pls[0], "http://kusc-ice.streamguys.com:80/kusc-128k");

	pls.clear();
	ASSERT_FALSE(m3u_decoder.decode(content_type_uri_list1,bad_uri_playlist, pls));
	ASSERT_TRUE(m3u_decoder.decode(content_type_uri_list2,playlist, pls));
	EXPECT_EQ(pls.size(), size_t(2));
	EXPECT_EQ(pls[0], "http://kusc-ice.streamguys.com:80/kusc-128k");
}
