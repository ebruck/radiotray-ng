// Copyright 2019 Edward G. Bruck <ed.bruck1@gmail.com>
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

#include <radiotray-ng/playlist/ds_decoder.hpp>
#include <gtest/gtest.h>


TEST(DsDecoder, test_that_url_is_detected_as_a_direct_stream)
{
    DsDecoder ds_decoder;

    ASSERT_TRUE(ds_decoder.is_url_direct_stream("http://example.com/stream.m3u8"));
    ASSERT_TRUE(ds_decoder.is_url_direct_stream("http://example.com/stream.M3u8"));
    ASSERT_TRUE(ds_decoder.is_url_direct_stream("http://example.com/stream.mp3"));
    ASSERT_TRUE(ds_decoder.is_url_direct_stream("http://example.com/stream.mP3"));
}
