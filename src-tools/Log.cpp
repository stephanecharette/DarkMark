/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


void dm::Log(const std::string & str)
{
	if (not str.empty())
	{
		char buffer[50];
		std::time_t tt = std::time(nullptr);
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", std::localtime(&tt));

		std::cout << buffer << str << std::endl;
	}

	return;
}
