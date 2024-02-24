// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class CfgHandler final
	{
		public:

			CfgHandler();
			~CfgHandler();

			/// Import/read/parse the template .cfg file, and store the contents in @ref v.
			CfgHandler & parse(const std::string & filename);

			/// Find the index of all the sections named.
			VSizet find_section(std::string name);

			/// Given a starting index, find the last index before EOF or the start of a new section.
			size_t find_end_of_section(const size_t start_of_section);

			/// Find the specified key, or return @p std::string::npos if not found.
			size_t find_key_in_section(const std::string & section, const std::string & key);

			/// Find the specified key, or return @p std::string::npos if not found.
			size_t find_key_in_section(const size_t start_of_section, const std::string & key);

			size_t set_or_add_line_in_section(const size_t start_of_section, const std::string & key, const std::string & val);

			/// Modify all instances of the named section so they contain the given key value pairs in @p m.
			CfgHandler & modify_all_sections(const std::string & name, const MStr & m);

			/// Count the number of anchors in the first [yolo] section.
			size_t number_of_anchors_in_yolo();

			/// Find the key in the given section, and return the value for that key.
			float get_value(const size_t start_of_section, const std::string & key);

			/// Return the value in the key-value pair at the given index.
			float get_value(const size_t idx);

			CfgHandler & fix_filters_before_yolo();

			/// Save the configuration to the given name.
			CfgHandler & output(const ProjectInfo & info);

			/// The entire .cfg file is stored in this vector until it is time to write it out.
			VStr cfg;

			/// Regex to help us find keys in the .cfg file.
			const std::regex key_value_rx;
	};
}
