/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/// Exact same as @p TabbedComponent, but changes the colour of the active tab to make it more obvious which is selected.
	class Notebook : public TabbedComponent
	{
		public:

			Notebook();
			virtual ~Notebook();
			virtual void currentTabChanged(int newCurrentTabIndex, const String & newCurrentTabName);
	};
}
