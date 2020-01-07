/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMCanvas : public CrosshairComponent
	{
		public:

			DMCanvas(DMContent & c);

			virtual ~DMCanvas();

			virtual void rebuild_cache_image();

			virtual void mouseDown(const MouseEvent & event);

			virtual void mouseDoubleClick(const MouseEvent & event);

			virtual void mouseDragFinished(juce::Rectangle<int> drag_rect);

			/// Link to the parent which manages the content, including all the marks.
			DMContent & content;
	};
}
