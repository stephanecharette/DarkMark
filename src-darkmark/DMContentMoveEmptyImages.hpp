// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentMoveEmptyImages : public ThreadWithProgressWindow
	{
		public:

			DMContentMoveEmptyImages(dm::DMContent & c);

			virtual ~DMContentMoveEmptyImages();

			virtual void run();

			DMContent & content;
	};
}
