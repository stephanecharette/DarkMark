// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::Cfg::Cfg(void) :
	PropertiesFile(
		[]
		{
			PropertiesFile::Options opt;

			opt.storageFormat			= PropertiesFile::StorageFormat::storeAsXML;
			opt.applicationName			= "DarkMark";
			opt.filenameSuffix			= ".cfg";
			opt.commonToAllUsers		= false;
			opt.ignoreCaseOfKeyNames	= true;

			return opt;
		}() )
{
	Log("configuration file used: " + getFile().getFullPathName().toStdString());

	first_time_initialization();

	load_all();

	return;
}


dm::Cfg::~Cfg(void)
{
	return;
}


dm::Cfg & dm::Cfg::first_time_initialization(void)
{
	std::string home = "/tmp";
	const char * ptr = getenv("HOME");
	if (ptr != nullptr)
	{
		home = ptr;
	}

	insert_if_not_exist("darknet_dir"					, home + "/darknet"									); // no longer used, see the new value "darknet_executable"

	// note where image_regex is used, the option "std::regex::icase" also gets set, so don't worry about uppercase/lowercase
	insert_if_not_exist("image_regex"					, "^.+\\.(?:(?:jpe?g)|(?:bmp)|(?:png)|(?:webp)|(?:tiff?)|(?:gif))$");

	insert_if_not_exist("sort_order"					, static_cast<int>(ESort::kAlphabetical)			);
	insert_if_not_exist("show_labels"					, static_cast<int>(EToggle::kAuto)					);
	insert_if_not_exist("show_predictions"				, static_cast<int>(EToggle::kAuto)					);
	insert_if_not_exist("show_marks"					, true												);
	insert_if_not_exist("alpha_blend_percentage"		, 65												);
	insert_if_not_exist("shade_rectangles"				, true												);
	insert_if_not_exist("all_marks_are_bold"			, false												);
	insert_if_not_exist("show_processing_time"			, true												);
	insert_if_not_exist("darknet_threshold"				, 50												); // https://www.ccoderun.ca/DarkHelp/api/classDarkHelp.html#a7e956a7d74f8d576e573da4ea92310f1
	insert_if_not_exist("darknet_hierarchy_threshold"	, 50												); // https://www.ccoderun.ca/DarkHelp/api/classDarkHelp.html#a7766c935160b80d696e232067afe8430
	insert_if_not_exist("darknet_nms_threshold"			, 45												); // https://www.ccoderun.ca/DarkHelp/api/classDarkHelp.html#ac533cda5d4cbba691deb4df5d89da318
	insert_if_not_exist("darknet_image_tiling"			, false												);
	insert_if_not_exist("review_table_row_height"		, 75												);
	insert_if_not_exist("scrollfield_width"				, 100												);
	insert_if_not_exist("scrollfield_marker_size"		, 7													);
	insert_if_not_exist("show_mouse_pointer"			, false												);
	insert_if_not_exist("corner_size"					, 10												);
	insert_if_not_exist("review_resize_thumbnails"		, true												);
	insert_if_not_exist("black_and_white_mode_enabled"	, false												);
	insert_if_not_exist("black_and_white_threshold_blocksize", 15											);
	insert_if_not_exist("black_and_white_threshold_constant", 15.0											);
	insert_if_not_exist("snap_horizontal_tolerance"		, 5													);
	insert_if_not_exist("snap_vertical_tolerance"		, 1													);
	insert_if_not_exist("snapping_enabled"				, false												);
	insert_if_not_exist("dilate_erode_mode"				, 0													);
	insert_if_not_exist("dilate_kernel_size"			, 2													);
	insert_if_not_exist("dilate_iterations"				, 1													);
	insert_if_not_exist("erode_kernel_size"				, 2													);
	insert_if_not_exist("erode_iterations"				, 1													);
	insert_if_not_exist("move_to_next_image_after_n"	, true												);

	insert_if_not_exist("heatmap_enabled"				, false												);
	insert_if_not_exist("heatmap_alpha_blend"			, 0.5												);
	insert_if_not_exist("heatmap_threshold"				, 0.1												);
	insert_if_not_exist("heatmap_visualize"				, 2													);

	// see at the bottom of this method where these two are initialized
	insert_if_not_exist("darknet_executable"			, ""												);
	insert_if_not_exist("darknet_templates"				, ""												);

	removeValue("darknet_enable_hue");	// this was changed to the float value darknet_hue
	removeValue("darknet_trailing_percentage");	// typo:  "trailing" -> "training"
	removeValue("darknet_enable_yolov3_tiny");	// this was changed to project-specific template
	removeValue("darknet_enable_yolov3_full");	// this was changed to project-specific template
	removeValue("crosshair_colour"); // crosshairs are now the colour of the last-used class
	removeValue("heatmap_sigma"); // this was changed to heatmap_threshold

	// global settings that have since been converted to be per-project
	removeValue("darknet_config"				);
	removeValue("darknet_weights"				);
	removeValue("darknet_names"					);
	removeValue("image_directory"				);
	removeValue("darknet_train_with_all_images"	);
	removeValue("darknet_training_percentage"	);
	removeValue("darknet_image_size"			);
	removeValue("darknet_batch_size"			);
	removeValue("darknet_subdivisions"			);
	removeValue("darknet_iterations"			);
	removeValue("darknet_resume_training"		);
	removeValue("darknet_delete_temp_weights"	);
	removeValue("darknet_saturation"			);
	removeValue("darknet_exposure"				);
	removeValue("darknet_hue"					);
	removeValue("darknet_enable_flip"			);
	removeValue("darknet_angle"					);
	removeValue("darknet_mosaic"				);
	removeValue("darknet_cutmix"				);
	removeValue("darknet_mixup"					);

	// Bug fixed in v1.1.0-3055 where these settings would get saved as zero if a neural network wasn't loaded.
	// This would cause serious problems next time DarkMark was loaded with a neural network.
	// If we see configurations out there set this way then fix it before the user notices.
	if (get_int("darknet_threshold"				) == 0 and
		get_int("darknet_hierarchy_threshold"	) == 0 and
		get_int("darknet_nms_threshold"			) == 0)
	{
		setValue("darknet_threshold"			, "50");
		setValue("darknet_hierarchy_threshold"	, "50");
		setValue("darknet_nms_threshold"		, "45");
	}

	// note where image_regex is used, the option "std::regex::icase" also gets set, so don't worry about uppercase/lowercase

	// update the image extension list to include webp files which had previously been missed (2022-09-03)
	if (get_str("image_regex") == "^.+\\.(?:(?:jpe?g)|(?:bmp)|(?:png)|(?:gif))$") // the old value
	{
		setValue("image_regex", "^.+\\.(?:(?:jpe?g)|(?:bmp)|(?:png)|(?:webp)|(?:gif))$");
	}

	// update the image extension list to include TIFF files
	if (get_str("image_regex") == "^.+\\.(?:(?:jpe?g)|(?:bmp)|(?:png)|(?:webp)|(?:gif))$")
	{
		setValue("image_regex", "^.+\\.(?:(?:jpe?g)|(?:bmp)|(?:png)|(?:webp)|(?:tiff?)|(?:gif))$");
	}

	// see if the (old-style) darknet directory exists, and if not, see if we can quickly find it
	auto darknet_dir = get_str("darknet_dir");
	File f(darknet_dir);
	if (f.exists() == false or f.isDirectory() == false)
	{
		// spend a few seconds looking through the "home" to see if we can find a subdirectory called "darknet"
		bool found = false;
		std::vector<File> dirs_to_search = {File(home)};

#ifdef WIN32
		// Linux is easy, but in Windows the darknet directory could be anywhere.  Queue up some
		// likely places we can try and search.  Remember we pop from the back, so put the better
		// options at the end of the list so we check those first.
		//
		dirs_to_search.push_back(File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory));
		dirs_to_search.push_back(File("/darknet"));
		dirs_to_search.push_back(File("/vcpkg"));
		dirs_to_search.push_back(File("/src"));
		dirs_to_search.push_back(File("/dev"));
		//
		// ...and we didn't check for C:, D:, etc..., so if we don't find Darknet
		// we'll rely on the user setting it up correctly in the .cfg file.
#endif
		const std::time_t start_time	= std::time(nullptr);
		const std::time_t end_time		= start_time + 5;
		while (found == false and dirs_to_search.empty() == false and std::time(nullptr) < end_time)
		{
			f = dirs_to_search.back();
			dirs_to_search.pop_back();
			dm::Log("looking for \"darknet\" in " + f.getFullPathName().toStdString());

			if (not f.exists())
			{
				continue;
			}
			if (not f.isDirectory())
			{
				continue;
			}

			// get the next level of subdirectories
			auto dirs = f.findChildFiles(File::TypesOfFileToFind::findDirectories + File::TypesOfFileToFind::ignoreHiddenFiles, false);
			for (const auto & child : dirs)
			{
				if (child.getFileName().toStdString().find("darknet") != std::string::npos)
				{
					// if this really is darknet, then it will have a subdirectory called "cfg"
					File darknet_cfg = child.getChildFile("cfg");
					if (darknet_cfg.isDirectory())
					{
						darknet_dir = child.getFullPathName().toStdString();
						dm::Log("found \"darknet\": " + darknet_dir);
						set_str("darknet_dir", darknet_dir);
						found = true;
						break;
					}
					else
					{
						dm::Log("found \"darknet\" but it doesn't have a \"cfg\" subdirectory: " + child.getFullPathName().toStdString());
					}
				}
				dirs_to_search.push_back(child);
			}
		}
	}

	auto darknet_executable = get_str("darknet_executable");
	f = File(darknet_executable);
	if (f.exists() == false)
	{
		dm::Log("darknet executable not found: \"" + darknet_executable + "\"");

#ifdef WIN32
		f = File("C:\\Program Files\\Darknet\\bin\\Darknet.exe");
#else
		f = File("/usr/bin/darknet");
#endif
		if (f.exists())
		{
			darknet_executable = f.getFullPathName().toStdString();
			dm::Log("setting darknet executable to " + darknet_executable);
		}
		else
		{
			dm::Log("cannot find \"" + darknet_executable + "\"");
			darknet_executable = "";

			// see if we can use the old darknet_dir value to guess where darknet is located
			if (darknet_dir.empty() == false)
			{
				f = File(darknet_dir).getChildFile("darknet");
				if (f.exists())
				{
					// this is the old-style build location, such as ~/src/darknet/darknet
					darknet_executable = f.getFullPathName().toStdString();
					dm::Log("darknet executable defaulting to " + darknet_executable);
				}
				else
				{
					// last-ditch attempt -- is this a new repo, but they didn't install the package yet?
					f = File(darknet_dir).getChildFile("build").getChildFile("src").getChildFile("darknet");
					if (f.exists())
					{
						darknet_executable = f.getFullPathName().toStdString();
						dm::Log("darknet executable in \"build\" directory: " + darknet_executable);
					}
				}
			}
			else
			{
				darknet_executable = "NOT_FOUND";
			}
		}

		set_str("darknet_executable", darknet_executable);
	}

	auto darknet_templates = get_str("darknet_templates");
	f = File(darknet_templates);
	if (f.exists() == false)
	{
		dm::Log("darknet config templates not found: \"" + darknet_templates + "\"");

#ifdef WIN32
		f = File("C:\\Program Files\\Darknet\\cfg\\");
#else
		f = File("/opt/darknet/cfg");
#endif
		if (f.exists())
		{
			darknet_templates = f.getFullPathName().toStdString();
			dm::Log("setting darknet config templates to " + darknet_templates);
		}
		else
		{
			dm::Log("cannot find \"" + darknet_templates + "\"");
			darknet_templates = "";

			// see if we can use the old darknet_dir value to find the .cfg files
			if (darknet_dir.empty() == false)
			{
				f = File(darknet_dir).getChildFile("cfg");
				if (f.isDirectory())
				{
					darknet_templates = f.getFullPathName().toStdString();
					dm::Log("darknet config templates defaulting to " + darknet_templates);
				}
			}
			else
			{
				darknet_templates = "NOT_FOUND";
			}
		}

		set_str("darknet_templates", darknet_templates);
	}

	saveIfNeeded();

	return *this;
}


dm::Cfg & dm::Cfg::load_all(void)
{
	return *this;
}


dm::Cfg & dm::Cfg::insert_if_not_exist(const std::string &key, const std::string &val)
{
	if (containsKey(key.c_str()) == false)
	{
		setValue(key.c_str(), val.c_str());
	}

	return *this;
}


dm::Cfg & dm::Cfg::insert_if_not_exist(const std::string &key, const int &val)
{
	return insert_if_not_exist(key, std::to_string(val));
}


dm::Cfg & dm::Cfg::insert_if_not_exist(const std::string &key, const double &val)
{
	return insert_if_not_exist(key, std::to_string(val));
}


dm::Cfg & dm::Cfg::set_str(const std::string &key, const std::string &val)
{
	setValue(key.c_str(), val.c_str());

	return *this;
}


std::string dm::Cfg::get_str(const std::string &key)
{
	if (containsKey(key.c_str()) == false)
	{
		throw std::invalid_argument("expected configuration to contain a key named \"" + key + "\"");
	}

	return getValue(key.c_str()).toStdString();
}


std::string dm::Cfg::get_str(const std::string &key, const std::string &default_value)
{
	if (containsKey(key.c_str()) == false)
	{
		return default_value;
	}

	return getValue(key.c_str()).toStdString();
}


int dm::Cfg::get_int(const std::string &key, const int default_value)
{
	if (containsKey(key.c_str()) == false)
	{
		return default_value;
	}

	return getIntValue(key.c_str());
}


double dm::Cfg::get_double(const std::string &key, const double default_value)
{
	if (containsKey(key.c_str()) == false)
	{
		return default_value;
	}

	return getDoubleValue(key.c_str());
}


bool dm::Cfg::get_bool(const std::string &key, const bool default_value)
{
	if (containsKey(key.c_str()) == false)
	{
		return default_value;
	}

	/* 2018-03-13:  There is a method called getBoolValue(), but when I tested
	 * it with values in configuration such as "true", it was returning false
	 * which led me to decide to write my own get_bool().
	 */

	const String val = getValue(key.c_str());

	if (val.compareIgnoreCase("true")	== 0 ||
		val.compareIgnoreCase("yes")	== 0 ||
		val.compareIgnoreCase("on")		== 0 ||
		val.compareIgnoreCase("t")		== 0 ||
		val.compareIgnoreCase("y")		== 0 ||
		val.compareIgnoreCase("1")		== 0)
	{
		return true;
	}

	if (val.compareIgnoreCase("false")	== 0 ||
		val.compareIgnoreCase("no")		== 0 ||
		val.compareIgnoreCase("off")	== 0 ||
		val.compareIgnoreCase("f")		== 0 ||
		val.compareIgnoreCase("n")		== 0 ||
		val.compareIgnoreCase("0")		== 0)
	{
		return false;
	}

	// otherwise, any other value is ignored and we return the default one
	return default_value;
}
