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

			DMContentRotateImages(dm::DMContent & c);

			virtual ~DMContentRotateImages();

			virtual void run();

			DMContent & content;
	};
}
