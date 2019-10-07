/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
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
	insert_if_not_exist("darknet_config"		, "stone_barcodes_yolov3-tiny.cfg"			);
	insert_if_not_exist("darknet_weights"		, "stone_barcodes_yolov3-tiny_final.weights");
	insert_if_not_exist("darknet_names"			, "stone_barcodes.names"					);
	insert_if_not_exist("image_directory"		, "/home/stephane/mailboxes"				);
	insert_if_not_exist("image_regex"			, "^.+\\.(?:(?:jpe?g)|(?:png)|(?:gif))$"	);
	insert_if_not_exist("sort_order"			, static_cast<int>(ESort::kAlphabetical)	);
	insert_if_not_exist("show_labels"			, static_cast<int>(EToggle::kAuto)			);
	insert_if_not_exist("alpha_blend_percentage", 35										);

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
