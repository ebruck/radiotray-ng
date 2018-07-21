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

#include "radiotray_ng.hpp"
#include <radiotray-ng/common.hpp>
#include <radiotray-ng/pidfile.hpp>
#include <radiotray-ng/event_bus/event_bus.hpp>
#include <radiotray-ng/player/player.hpp>
#include <radiotray-ng/bookmarks/bookmarks.hpp>
#include <radiotray-ng/config/config.hpp>
#include <radiotray-ng/extras/media_keys/media_keys.hpp>
#include <rtng_user_agent.hpp>

#ifdef APPINDICATOR_GUI
#include <radiotray-ng/gui/appindicator/appindicator_gui.hpp>
#else
#include <radiotray-ng/gui/ncurses/ncurses_gui.hpp>
#endif

#ifdef RTNG_DBUS
#include <radiotray-ng/extras/rtng_dbus/rtng_dbus.hpp>
#endif

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/program_options.hpp>


void init_logging()
{
	std::string xdg_data_home_dir = radiotray_ng::get_data_dir(APP_NAME);

	namespace keywords = boost::log::keywords;

	auto sink = boost::log::add_file_log
		(
			keywords::file_name = xdg_data_home_dir + "logs/radiotray-ng-%5N.log",
			keywords::rotation_size = 1024 * 64, // 64K logs
			keywords::open_mode = std::ios_base::app,
			keywords::auto_flush = true,
			keywords::format =
				(
					boost::log::expressions::stream
					<< boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "[%Y-%m-%d %H:%M:%S.%f]")
					<< " [" << boost::log::expressions::attr< boost::log::attributes::current_thread_id::value_type >("ThreadID")
					<< "] [" << std::setw(5) << std::left << boost::log::trivial::severity << "] " <<  boost::log::expressions::smessage
				)
		);

	boost::log::add_common_attributes();

	sink->locked_backend()->set_file_collector(boost::log::sinks::file::make_collector
		(
			keywords::target = xdg_data_home_dir + "logs/",
			keywords::max_size = 1024 * 512 // ~512K of logs
		));

	sink->locked_backend()->scan_for_files();

	boost::log::core::get()->add_sink(sink);
}


void set_logging_level(std::shared_ptr<IConfig>& config)
{
	if (config->get_bool(DEBUG_LOGGING_KEY, DEFAULT_DEBUG_LOGGING_VALUE))
	{
		LOG(info) << "debug logging enabled";

		boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
	}
	else
	{
		LOG(info) << "debug logging disabled";

		boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::debug);
	}
}


void set_config_defaults(std::shared_ptr<IConfig>& config, const std::string& config_path)
{
	namespace fs = boost::filesystem;

	std::string bookmarks_file{config_path + RTNG_BOOKMARK_FILE};

	// something for the user to edit...
	config->set_string(BOOKMARKS_KEY, bookmarks_file);
	config->set_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE);
	config->set_bool(SPLIT_TITLE_KEY, DEFAULT_SPLIT_TITLE_VALUE);
	config->set_uint32(VOLUME_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_VALUE);
	config->set_bool(NOTIFICATION_VERBOSE_KEY, DEFAULT_NOTIFICATION_VERBOSE_VALUE);

	// copy over included bookmarks file if none exists...
	if (!fs::exists(bookmarks_file))
	{
		fs::copy(RTNG_DEFAULT_BOOKMARK_FILE, bookmarks_file);
	}

	config->save();
}


void set_bookmark_defaults(std::shared_ptr<IBookmarks>& bookmarks)
{
	bookmarks->add_group(ROOT_BOOKMARK_GROUP, DEFAULT_STATION_IMAGE_VALUE);
}


bool create_data_dir(std::string& config_path)
{
	namespace fs = boost::filesystem;

	std::string xdg_data_home_dir = radiotray_ng::get_data_dir(APP_NAME);

	if (!fs::exists(xdg_data_home_dir))
	{
		if (!fs::create_directory(xdg_data_home_dir))
		{
			std::cerr << "failed to create dir: " << xdg_data_home_dir;
			return false;
		}
	}

	config_path = xdg_data_home_dir;

	return true;
}


bool process_command_line_args(int argc, char* argv[])
{
	try
	{
		namespace po = boost::program_options;

		po::options_description desc("Options");

		desc.add_options()
			("help",    "Shows this information")
			("version", "Show the application's version")
			("play",    "Resume playback");

		po::variables_map vm;

		try
		{
			po::store(po::parse_command_line(argc, argv, desc), vm);

			if (vm.count("help"))
			{
				std::cout << "Usage:" << std::endl
					<< "  " << APP_NAME << " [OPTION]" << std::endl
					<< std::endl << desc << std::endl;
				return false;
			}

			if (vm.count("version"))
			{
				// same as about box...
				std::string version("v" RTNG_VERSION);

				// if git version differs, then append hash...
				if (version != RTNG_GIT_VERSION)
				{
					version += " (" RTNG_GIT_VERSION ")";
				}

				std::cout << APP_NAME_DISPLAY << ": " << version << std::endl;
				return false;
			}

			po::notify(vm);
		}
		catch(po::error& e)
		{
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			std::cerr << desc << std::endl;
			return false;
		}
	}
	catch(std::exception& e)
	{
		std::cerr << "Unhandled Exception: " << e.what() << ", application will now exit" << std::endl;
		return false;
	}

	return true;
}


int main(int argc, char* argv[])
{
	if (!process_command_line_args(argc, argv))
	{
		return 0;
	}

	std::string config_path;
	if (!create_data_dir(config_path))
	{
		return 1;
	}

	radiotray_ng::Pidfile pf(APP_NAME);

	if (pf.is_running())
	{
		return 1;
	}

	init_logging();

	LOG(info) << APP_NAME << " (" << RTNG_GIT_VERSION << ") starting up";

	std::shared_ptr<IConfig> config = std::make_shared<Config>(config_path + RTNG_CONFIG_FILE);

	// load config or create a new one
	if (!config->load())
	{
		// not there, so lets set some defaults...
		set_config_defaults(config, config_path);
	}

	// adjust logging level...
	set_logging_level(config);

	std::shared_ptr<IBookmarks> bookmarks = std::make_shared<Bookmarks>(config->get_string(BOOKMARKS_KEY, RTNG_DEFAULT_BOOKMARK_FILE));
	std::shared_ptr<IEventBus> event_bus = std::make_shared<EventBus>();
	std::shared_ptr<IPlayer> player = std::make_shared<Player>(config, event_bus);

	if (!bookmarks->load())
	{
		// User can reload once they fix the error...
		set_bookmark_defaults(bookmarks);
	}

	auto radiotray_ng = std::make_shared<RadiotrayNG>(config, bookmarks, player, event_bus);

#ifdef APPINDICATOR_GUI
	auto gui = std::make_shared<AppindicatorGui>(config, radiotray_ng, bookmarks, event_bus);
#else
	auto gui = std::make_shared<NCursesGui>(radiotray_ng, event_bus);
#endif

#ifdef RTNG_DBUS
	RtngDbus dbus(gui, radiotray_ng);
#endif

	// addons etc.
	MediaKeys mm(radiotray_ng, config);

	gui->run(argc, argv);

	LOG(info) << APP_NAME << " shutting down";

	return 0;
}
