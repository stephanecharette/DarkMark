// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentStatistics : public ThreadWithProgressWindow
	{
		public:

			DMContentStatistics(dm::DMContent & c);

			virtual ~DMContentStatistics();

			virtual void run();

			DMContent & content;
	};
}
