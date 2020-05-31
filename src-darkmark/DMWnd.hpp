/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/// Main DarkMark window.  Users can then mark up images and access several secondary windows.
	class DMWnd : public DocumentWindow
	{
		public:

			DMWnd(const std::string & prefix);

			virtual ~DMWnd(void);

			virtual void closeButtonPressed();

			DMContent content;
	};
}
