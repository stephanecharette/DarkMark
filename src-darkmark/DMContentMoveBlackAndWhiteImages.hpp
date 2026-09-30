// DarkMark (C) 2019-2026 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentMoveBlackAndWhiteImages : public ThreadWithProgressWindow
	{
		public:

			DMContentMoveBlackAndWhiteImages(dm::DMContent & c);

			virtual ~DMContentMoveBlackAndWhiteImages();

			virtual void run();

			DMContent & content;
	};
}
