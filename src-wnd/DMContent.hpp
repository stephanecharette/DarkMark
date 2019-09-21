/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContent : public Component
	{
		public:

			DMContent();

			virtual ~DMContent();

			virtual void resized();

			DMCanvas canvas;
			DMCorner corner[4];
	};
}
