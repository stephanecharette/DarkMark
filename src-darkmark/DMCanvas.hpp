// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/** This is the actual class that draws the current image and all of the annotations/marks.  Most of the work is performed
	 * in @ref rebuild_cache_image().  Also of importance is the mouse event handling to ensure that marks are created and
	 * stretched correctly.
	 *
	 * This class is one of the children of @ref DMContent.
	 */
	class DMCanvas : public CrosshairComponent
	{
		public:

			DMCanvas(DMContent & c);

			virtual ~DMCanvas();

			virtual void rebuild_cache_image();

			virtual void mouseDown(const MouseEvent & event) override;
			virtual void mouseDoubleClick(const MouseEvent & event) override;
			virtual void mouseDragFinished(juce::Rectangle<int> drag_rect) override;

			/// Link to the parent which manages the content, including all the marks.
			DMContent & content;
	};
}
