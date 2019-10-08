/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentStatistics : public ThreadWithProgressWindow
	{
		public:

			DMContentStatistics(dm::DMContent & c);

			virtual void run();

			DMContent & content;
	};
}
