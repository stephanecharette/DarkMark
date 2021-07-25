// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


void dm::Log(const std::string & str)
{
	if (not str.empty())
	{
		char buffer[50];
		std::time_t tt = std::time(nullptr);
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", std::localtime(&tt));

		std::cout << buffer << str << std::endl;

		static std::ofstream ofs("/tmp/darkmark.log", std::ofstream::trunc);
		if (ofs.is_open())
		{
			ofs << buffer << str << std::endl << std::flush;
		}
	}

	return;
}
