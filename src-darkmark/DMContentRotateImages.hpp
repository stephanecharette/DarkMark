/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

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
