// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::CfgHandler::CfgHandler() :
	key_value_rx(
			"^"			// start of text
			"[ \t]*"	// optional whitespace
			"("			// 1st group
			"[^# \t=]+"	// KEY (no comment, whitespace, or "=" sign)
			")"
			"[ \t]*"	// optional whitespace
			"="			// "="
			"[ \t]*"	// optional whitespace
			"("			// 2nd group
			".*"		// VALUE
			")"
			"$")		// end of text
{
	return;
}


dm::CfgHandler::~CfgHandler()
{
	return;
}


dm::CfgHandler & dm::CfgHandler::parse(const std::string & filename)
{
	cfg.clear();

	Log("CfgHandler: loading configuration file \"" + filename + "\"");

	std::ifstream ifs(filename);
	if (ifs.good() == false)
	{
		throw std::runtime_error("Failed to read the template configuration file " + filename + ".");
	}

	std::string line;
	while (std::getline(ifs, line))
	{
		cfg.push_back(line);
	}
	ifs.close();

	if (cfg.empty())
	{
		throw std::runtime_error("Something is wrong, the configuration file " + filename + " appears to be empty.");
	}

	const auto v = find_section("[net]");
	if (v.empty())
	{
		throw std::runtime_error("Expected the configuration file " + filename + " to start with a [net] section.");
	}
	if (v.size() > 1)
	{
		throw std::runtime_error("Expected the configuration file " + filename + " to contain a single [net] section.");
	}

	Log("CfgHandler: loaded " + std::to_string(cfg.size() + 1) + " lines from configuration file \"" + filename + "\"");

	return *this;
}


dm::VSizet dm::CfgHandler::find_section(std::string name)
{
	if (name.empty())					throw std::invalid_argument("find_section() requires a valid name");
	if (name[0] != '[')					name = "[" + name;
	if (name[name.length() - 1] != ']')	name += "]";

	VSizet v;

	Log("looking for \"" + name + "\" in " + std::to_string(cfg.size() + 1) + " lines");

	for (size_t idx = 0; idx < cfg.size(); idx ++)
	{
		const std::string & line = cfg.at(idx);
		if (line == name)
		{
			Log("-> found \"" + name + "\" at index #" + std::to_string(idx));
			v.push_back(idx);
		}
	}

	return v;
}


size_t dm::CfgHandler::find_end_of_section(const size_t start_of_section)
{
	if (start_of_section >= cfg.size())
	{
		throw std::invalid_argument("cannot find end-of-section for index #" + std::to_string(start_of_section) + " when the configuration file has only " + std::to_string(cfg.size() + 1) + " lines");
	}

	size_t last_non_empty_line = start_of_section;

	for (size_t idx = start_of_section + 1; idx < cfg.size(); idx ++)
	{
		const std::string & line = cfg.at(idx);

		if (line.empty())
		{
			continue;
		}
		if (line[0] == '[')
		{
			break;
		}
		last_non_empty_line = idx;
	}

	Log("section that begins at index #" + std::to_string(start_of_section) + " ends at index #" + std::to_string(last_non_empty_line));

	return last_non_empty_line;
}


size_t dm::CfgHandler::find_key_in_section(const std::string & section, const std::string & key)
{
	const auto v = find_section(section);
	for (const auto section_idx : v)
	{
		const auto idx = find_key_in_section(section_idx, key);
		if (idx != std::string::npos)
		{
			return idx;
		}
	}

	// otherwise the key was not found
	return std::string::npos;
}


size_t dm::CfgHandler::find_key_in_section(const size_t start_of_section, const std::string & key)
{
	if (start_of_section >= cfg.size())
	{
		throw std::invalid_argument("cannot find key \"" + key + "\" at index #" + std::to_string(start_of_section) + " when the configuration file has only " + std::to_string(cfg.size() + 1) + " lines");
	}

	const size_t end_of_section = find_end_of_section(start_of_section);

	Log("looking between indexes #" + std::to_string(start_of_section) + " and " + std::to_string(end_of_section) + " for key \"" + key + "\"");

	for (size_t idx = start_of_section + 1; idx <= end_of_section; idx ++)
	{
		std::string & line = cfg.at(idx);
		std::smatch what;
		const bool valid = std::regex_match(line, what, key_value_rx);
		if (valid)
		{
			const std::string name = what.str(1);
			if (name == key)
			{
				return idx;
			}
		}
	}

	return std::string::npos;
}


size_t dm::CfgHandler::set_or_add_line_in_section(const size_t start_of_section, const std::string & key, const std::string & val)
{
	if (start_of_section >= cfg.size())
	{
		throw std::invalid_argument("cannot find or add \"" + key + "=" + val + "\" at index #" + std::to_string(start_of_section) + " when the configuration file has only " + std::to_string(cfg.size() + 1) + " lines");
	}

	size_t idx = find_key_in_section(start_of_section, key);
	if (idx == std::string::npos)
	{
		// key does not exist, we need to insert a new line
		idx = find_end_of_section(start_of_section) + 1;
		cfg.insert(cfg.begin() + idx, "");
	}

	const std::string line = key + "=" + val;
	Log("at index #" + std::to_string(idx) + ": \"" + cfg[idx] + "\" -> \"" + line + "\"");
	cfg[idx] = line;

	return idx;
}


dm::CfgHandler & dm::CfgHandler::modify_all_sections(const std::string & name, const MStr & m)
{
	const auto section_indexes = find_section(name);

	// because we may end up adding new lines, this will cause the .cfg file to grow and our
	// section indexes will be off, so make sure that we process the sections in reverse order

	for (auto iter = section_indexes.rbegin(); iter != section_indexes.rend(); iter ++)
	{
		const size_t idx = *iter;

		for (const auto & [key, val] : m)
		{
			set_or_add_line_in_section(idx, key, val);
		}
	}

	return *this;
}


size_t dm::CfgHandler::number_of_anchors_in_yolo()
{
	const size_t idx = find_key_in_section("[yolo]", "anchors");
	if (idx == std::string::npos)
	{
		Log("yolo section not found, or anchors haven't been defined");
		return 0;
	}

	/* The line could look like this:
	 *
	 *		anchors = 10,14,  23,27,  37,58,  81,82,  135,169,  344,319
	 *
	 * ...or it could look like this:
	 *
	 *		anchors = 12, 16, 19, 36, 40, 28, 36, 75, 76, 55, 72, 146, 142, 110, 192, 243, 459, 401
	 *
	 * That first example is from yolov4-tiny.cfg with 6 clusters.  The 2nd example is from yolov4.cfg with 9 clusters.
	 * So count the number of commas, add 1, and divide by 2 to get the number of anchors.  For example:
	 *
	 *		anchors = 10,14,  23,27,  37,58,  81,82,  135,169,  344,319
	 *
	 * ...has 11 commas.  +1 means 12.  12/2 = 6, meaning 6 pairs, thus cluster=6.
	 */

	const std::string & line = cfg.at(idx);
	size_t count = std::count(line.begin(), line.end(), ',');
	count ++;
	count /= 2;

	Log("yolo section contains " + std::to_string(count) + " anchors");

	return count;
}


dm::CfgHandler & dm::CfgHandler::output(const dm::ProjectInfo & info)
{
	if (cfg.empty())
	{
		Log("attempt to save empty configuration file to " + info.cfg_filename);
		throw std::runtime_error("cannot save empty configuration file to \"" + info.cfg_filename + "\"");
	}

	std::ofstream ofs(info.cfg_filename);
	if (ofs.good() == false)
	{
		// something is very wrong if we get here
		Log("cannot save configuration file " + info.cfg_filename + " (cfg=" + std::to_string(cfg.size()) + ", template=" + info.cfg_template + ")");
		throw std::runtime_error("cannot save configuration file \"" + info.cfg_filename + "\"");
	}

	ofs	<< "# DarkMark v" << DARKMARK_VERSION << " output for Darknet"																<< std::endl
		<< "# Project .... " << info.project_dir																					<< std::endl
		<< "# Config ..... " << info.cfg_filename																					<< std::endl
		<< "# Template ... " << info.cfg_template																					<< std::endl
		<< "# Username ... " << SystemStats::getLogonName().toStdString() << "@" << SystemStats::getComputerName().toStdString()	<< std::endl
		<< "# Timestamp .. " << Time::getCurrentTime().formatted("%a %Y-%m-%d %H:%M:%S %Z").toStdString()							<< std::endl
		<< "#"																														<< std::endl
		<< "# WARNING:  If you re-generate the darknet files for this project you'll"												<< std::endl
		<< "#           lose any customizations you are about to make in this file!"												<< std::endl
		<< "#"																														<< std::endl
		<< ""																														<< std::endl;

	for (const auto & line : cfg)
	{
		ofs << line << std::endl;
	}

	if (ofs.fail())
	{
		Log("error saving config file " + info.cfg_filename);
		throw std::runtime_error("failed to write configuration file \"" + info.cfg_filename + "\"");
	}

	return *this;
}


float dm::CfgHandler::get_value(const size_t start_of_section, const std::string & key)
{
	float val = 0.0f;

	const size_t idx = find_key_in_section(start_of_section, key);

	if (idx != std::string::npos)
	{
		val = get_value(idx);
	}

	return val;
}


float dm::CfgHandler::get_value(const size_t idx)
{
	if (idx >= cfg.size())
	{
		throw std::invalid_argument("cannot get the key-value pair at index #" + std::to_string(idx) + " when the configuration file has only " + std::to_string(cfg.size() + 1) + " lines");
	}

	float val = 0.0f;

	std::smatch what;
	const bool valid = std::regex_match(cfg.at(idx), what, key_value_rx);
	if (valid)
	{
		val = std::stof(what.str(2));
	}

	return val;
}


dm::CfgHandler & dm::CfgHandler::fix_filters_before_yolo()
{
	/* Several lines prior to [yolo] is a "filters=..." line that we need to fix.  It normally looks like this:
	 *
	 *		...
	 *		[convolutional]
	 *		size=1
	 *		stride=1
	 *		pad=1
	 *		filters=18
	 *		activation=linear
	 *
	 *		[yolo]
	 *		mask = 6,7,8
	 *		anchors = 12, 16, 19, 36, 40, 28, 36, 75, 76, 55, 72, 146, 142, 110, 192, 243, 459, 401
	 *		classes=1
	 *		num=9
	 *		...
	 *
	 * Note the "filters=18" about 3 lines prior to "[yolo]".  That is the line we need to find and fix.
	 * The value needs to be:
	 *
	 *		(number_of_classes + 5) * number_of_masks
	 *
	 * For most configurations (YOLOv4, YOLOv4-tiny, ...) the number_of_masks is typically 3.  But it can be different.
	 * The yolov4-tiny-contrastive.cfg file for example looks like this:
	 *
	 *		...
	 *		[convolutional]
	 *		size=1
	 *		stride=1
	 *		pad=1
	 *		filters=765
	 *		activation=linear
	 *
	 *		[yolo]
	 *		mask = 0,1,2,3,4,5,6,7,8
	 *		anchors = 10,13,  16,30,  33,23,  30,61,  62,45,  59,119,  116,90,  156,198,  373,326
	 *		classes=80
	 *		num=9
	 *		jitter=.3
	 *
	 * (classes[80] + 5) * number_of_masks[9] = 85 * 9 = 765.
	 */

	for (const auto yolo_idx : find_section("[yolo]"))
	{
		// we need the number of classes and the number of masks to calculate the filters
		const size_t idx_masks			= find_key_in_section(yolo_idx, "mask");
		const std::string & masks		= cfg.at(idx_masks);
		const size_t number_of_masks	= 1 + std::count(masks.begin(), masks.end(), ',');
		const size_t number_of_classes	= static_cast<size_t>(get_value(yolo_idx, "classes"	));
		const size_t filters			= (number_of_classes + 5) * number_of_masks;

		// we're going to go backwards from "[yolo]" to find the filters=... line, so first thing to do is locate the upper bound
		size_t end_idx = 0;
		if (yolo_idx > 20)
		{
			end_idx = yolo_idx - 20;
		}

		for (size_t idx = yolo_idx - 1; idx > end_idx; idx --)
		{
			std::string & line = cfg.at(idx);
			std::smatch what;
			const bool valid = std::regex_match(line, what, key_value_rx);
			if (valid)
			{
				const std::string name = what.str(1);
				if (name == "filters")
				{
					// replace this line
					line = "filters=" + std::to_string(filters);
					break;
				}
			}
		}
	}

	return *this;
}
