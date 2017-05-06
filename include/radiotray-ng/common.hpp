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

#pragma once

#include <boost/log/trivial.hpp>
#include <radiotray-ng/helpers.hpp>
#include <vector>
#include <string>
#include <iomanip>

using playlist_t = std::vector<std::string>;

// logging
#define LOG(x)                            BOOST_LOG_TRIVIAL(x) << "(" << radiotray_ng::path_to_filename(__FILE__) << ":"  << __LINE__ << ") - "

// event data keys
#define ERROR_KEY                         "error"
#define SONG_KEY                          "song"
#define STATE_KEY                         "state"
#define BOOKMARKS_KEY                     "bookmarks"

// state data values
#define STATE_BUFFERING                   "buffering"
#define STATE_PAUSED                      "paused"
#define STATE_PLAYING                     "playing"
#define STATE_STOPPED                     "stopped"
#define STATE_CONNECTING                  "connecting"

// tag data values
#define TAG_ARTIST                        "artist"
#define TAG_BITRATE                       "bitrate"
#define TAG_CODEC                         "audio codec"
#define TAG_TITLE                         "title"

// http codes
#define HTTP_OK                           (200)
#define HTTP_NOT_FOUND                    (404)

// config keys
#define BOOKMARKS_KEY                     "bookmarks"
#define BUFFER_SIZE_KEY                   "buffer-size"
#define COMPACT_MENU_KEY                  "compact-menu"
#define DEBUG_LOGGING_KEY                 "debug-logging"
#define HTTP_TIMEOUT_KEY                  "http-timeout"
#define LAST_STATION_GROUP_KEY            "last-station-group"
#define LAST_STATION_KEY                  "last-station"
#define MEDIA_KEY_MAPPING_KEY             "media-key-mapping"
#define MEDIA_KEY_NEXT_STAITON_KEY        "media-key-next-station"
#define MEDIA_KEY_PREVIOUS_STATION_KEY    "media-key-previous-station"
#define MEDIA_KEY_VOLUME_DOWN_KEY         "media-key-volume-down"
#define MEDIA_KEY_VOLUME_UP_KEY           "media-key-volume-up"
#define NOTIFICATION_IMAGE_KEY            "notification-image"
#define NOTIFICATION_KEY                  "notifications"
#define NOTIFICATION_VERBOSE_KEY          "notification-verbose"
#define SLEEP_TIMER_KEY                   "sleep-timer"
#define SPLIT_TITLE_KEY                   "split-title"
#define TAG_INFO_VERBOSE_KEY              "tag-info-verbose"
#define VOLUME_LEVEL_KEY                  "volume-level"
#define VOLUME_MAX_LEVEL_KEY              "volume-max-level"
#define VOLUME_STEP_KEY                   "volume-step"

// conf files etc.
#define RTNG_BOOKMARK_FILE                "bookmarks.json"
#define RTNG_CONFIG_FILE                  "radiotray-ng.json"
#define RTNG_DEFAULT_BOOKMARK_FILE        "/usr/share/radiotray-ng/bookmarks.json"
#define RTNG_DEFAULT_INSTALL_DIR          "/usr/share/radiotray-ng/"

// misc
#define ROOT_BOOKMARK_GROUP               "root"
#define APP_NAME_DISPLAY                  "Radiotray-NG"
#define APP_NAME                          "radiotray-ng"
#define APP_COPYRIGHT                     "Copyright (C) 2017 Edward G. Bruck"
#define APP_WEBSITE                       "https://www.github.com/ebruck/radiotray-ng"

// defaults
#define DEFAULT_BUFFER_SIZE_VALUE          uint32_t(128000*10)
#define DEFAULT_COMPACT_MENU_VALUE         (false)
#define DEFAULT_DEBUG_LOGGING_VALUE        (false)
#define DEFAULT_HTTP_TIMEOUT_VALUE         (15)
#define DEFAULT_MEDIA_KEY_MAPPING_VALUE    (false)
#define DEFAULT_MEDIA_KEY_NEXT_STATION_VALUE     "Next"
#define DEFAULT_MEDIA_KEY_PREVIOUS_STATION_VALUE "Previous"
#define DEFAULT_MEDIA_KEY_VOLUME_DOWN_VALUE      ""
#define DEFAULT_MEDIA_KEY_VOLUME_UP_VALUE        ""
#define DEFAULT_NOTIFICATION_IMAGE_VALUE   "radiotray-ng-notification"
#define DEFAULT_NOTIFICATION_VALUE         (true)
#define DEFAULT_NOTIFICATION_VERBOSE_VALUE (true)
#define DEFAULT_SLEEP_TIMER_VALUE          uint32_t(60)
#define DEFAULT_SPLIT_TITLE_VALUE          (true)
#define DEFAULT_STATION_IMAGE_VALUE        ""
#define DEFAULT_VOLUME_LEVEL_MAX_VALUE     uint32_t(200)
#define DEFAULT_VOLUME_LEVEL_VALUE         uint32_t(100)
#define DEFAULT_VOLUME_STEP_VALUE          uint32_t(1)

// icons
#define RADIOTRAY_NG_ICON_ON               "radiotray-ng-on"
#define RADIOTRAY_NG_ICON_OFF              "radiotray-ng-off"
#define RADIOTRAY_NG_LOGO_ICON             "radiotray-ng.png"

