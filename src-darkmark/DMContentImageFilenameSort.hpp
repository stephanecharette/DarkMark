/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentImageFilenameSort : public ThreadWithProgressWindow
	{
		public:

			DMContentImageFilenameSort(dm::DMContent & c);

			virtual void run();

			DMContent & content;
	};
}
