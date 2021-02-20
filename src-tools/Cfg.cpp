// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::Cfg::Cfg(void) :
	PropertiesFile(
		[]
		{
			PropertiesFile::Options options;

			options.storageFormat			= PropertiesFile::StorageFormat::storeAsXML;
			options.applicationName			= "DarkMark";
			options.filenameSuffix			= ".cfg";
			options.commonToAllUsers		= false;
			options.ignoreCaseOfKeyNames	= true;

			return options;
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

	insert_if_not_exist("darknet_dir"					, home + "/darknet"									);
	insert_if_not_exist("image_regex"					, "^.+\\.(?:(?:jpe?g)|(?:bmp)|(?:png)|(?:gif))$"	);
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

	removeValue("darknet_enable_hue");	// this was changed to the float value darknet_hue
	removeValue("darknet_trailing_percentage");	// typo:  "trailing" -> "training"
	removeValue("darknet_enable_yolov3_tiny");	// this was changed to project-specific template
	removeValue("darknet_enable_yolov3_full");	// this was changed to project-specific template
	removeValue("crosshair_colour"); // crosshairs are now the colour of the last-used class

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
