/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

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

	insert_if_not_exist("darknet_config"				, ""												);
	insert_if_not_exist("darknet_weights"				, ""												);
	insert_if_not_exist("darknet_names"					, ""												);
	insert_if_not_exist("darknet_dir"					, home + "/darknet"									);
	insert_if_not_exist("image_directory"				, home + "/nn/flowers"								);
	insert_if_not_exist("image_regex"					, "^.+\\.(?:(?:jpe?g)|(?:bmp)|(?:png)|(?:gif))$"	);
	insert_if_not_exist("sort_order"					, static_cast<int>(ESort::kAlphabetical)			);
	insert_if_not_exist("show_labels"					, static_cast<int>(EToggle::kAuto)					);
	insert_if_not_exist("show_predictions"				, static_cast<int>(EToggle::kAuto)					);
	insert_if_not_exist("show_marks"					, true												);
	insert_if_not_exist("alpha_blend_percentage"		, 65												);
	insert_if_not_exist("all_marks_are_bold"			, false												);
	insert_if_not_exist("show_processing_time"			, true												);
	insert_if_not_exist("darknet_enable_yolov3_tiny"	, true												);
	insert_if_not_exist("darknet_enable_yolov3_full"	, true												);
	insert_if_not_exist("darknet_trailing_percentage"	, 85												);
	insert_if_not_exist("darknet_image_size"			, 416												);
	insert_if_not_exist("darknet_batch_size"			, 64												);
	insert_if_not_exist("darknet_subdivisions"			, 8													);
	insert_if_not_exist("darknet_iterations"			, 4000												);
	insert_if_not_exist("darknet_saturation"			, 1.50f												);
	insert_if_not_exist("darknet_exposure"				, 1.50f												);
	insert_if_not_exist("darknet_hue"					, 0.10f												);
	insert_if_not_exist("darknet_enable_flip"			, true												);
	insert_if_not_exist("darknet_angle"					, 0													);
	insert_if_not_exist("darknet_mosaic"				, false												);
	insert_if_not_exist("darknet_cutmix"				, false												);
	insert_if_not_exist("darknet_mixup"					, false												);
	insert_if_not_exist("darknet_threshold"				, 50												); // https://www.ccoderun.ca/DarkHelp/api/classDarkHelp.html#a7e956a7d74f8d576e573da4ea92310f1
	insert_if_not_exist("darknet_hierarchy_threshold"	, 50												); // https://www.ccoderun.ca/DarkHelp/api/classDarkHelp.html#a7766c935160b80d696e232067afe8430
	insert_if_not_exist("darknet_nms_threshold"			, 45												); // https://www.ccoderun.ca/DarkHelp/api/classDarkHelp.html#ac533cda5d4cbba691deb4df5d89da318
	insert_if_not_exist("crosshair_colour"				, "ffffffff"										); // alpha + rgb
	insert_if_not_exist("review_table_row_height"		, 75												);

	removeValue("darknet_enable_hue");	// this was changed to the float value darknet_hue

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


int dm::Cfg::get_int(const std::string &key)
{
	if (containsKey(key.c_str()) == false)
	{
		throw std::invalid_argument("expected configuration to contain a key named \"" + key + "\"");
	}

	return getIntValue(key.c_str());
}


double dm::Cfg::get_double(const std::string &key)
{
	if (containsKey(key.c_str()) == false)
	{
		throw std::invalid_argument("expected configuration to contain a key named \"" + key + "\"");
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
