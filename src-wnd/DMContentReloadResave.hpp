/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

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
