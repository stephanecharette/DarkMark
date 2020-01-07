/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMWnd : public DocumentWindow
	{
		public:

			DMWnd(void);

			virtual ~DMWnd(void);

			virtual void closeButtonPressed();

			DMContent content;
	};
}
