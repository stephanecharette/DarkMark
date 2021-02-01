// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentRotateImages : public ThreadWithProgressWindow
	{
		public:

			DMContentRotateImages(dm::DMContent & c, const bool all_images);

			virtual ~DMContentRotateImages();

			virtual void run();

			DMContent & content;

			bool apply_to_all_images;
	};
}
