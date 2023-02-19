// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentReloadResave : public ThreadWithProgressWindow
	{
		public:

			DMContentReloadResave(dm::DMContent & c);

			virtual ~DMContentReloadResave();

			virtual void run();

			DMContent & content;
	};
}
