// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"

namespace dm
{
	enum class EToggle
	{
		kInvalid	= -1,
		kOff		= 0,
		kOn			= 1,
		kAuto		= 2
	};

	class Cfg final : public PropertiesFile
	{
		public:

			Cfg();

			virtual ~Cfg();

			virtual Cfg &first_time_initialization();

			virtual Cfg &load_all();

			virtual Cfg &insert_if_not_exist(const std::string &key, const std::string &val);
			virtual Cfg &insert_if_not_exist(const std::string &key, const int &val);
			virtual Cfg &insert_if_not_exist(const std::string &key, const double &val);

			virtual Cfg &set_str(const std::string &key, const std::string &val);

			/// The key must exist, otherwise an exception is thrown.
			virtual std::string get_str(const std::string &key);

			/// Return the corresponding value, or the default value if the key does not exist.
			virtual std::string get_str(const std::string &key, const std::string &default_value);

			/// The key must exist, otherwise an exception is thrown.
			virtual int get_int(const std::string &key, const int default_value=0);

			/// The key must exist, otherwise an exception is thrown.
			virtual double get_double(const std::string &key, const double default_value=0.0f);

			/// Return the corresponding value, or the default value if the key does not exist.
			virtual bool get_bool(const std::string &key, const bool default_value=true);
	};
}
