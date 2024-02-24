// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentResizeTLTR : public ThreadWithProgressWindow
	{
		public:

			DMContentResizeTLTR(dm::DMContent & c);

			virtual ~DMContentResizeTLTR();

			virtual void run();

			DMContent & content;
	};
}
