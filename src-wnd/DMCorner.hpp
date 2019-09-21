/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMCorner final : public Component
	{
		public:

			DMCorner();

			virtual ~DMCorner();

			virtual void resized();

			virtual void paint(Graphics & g);

			virtual void mouseMove(const MouseEvent & event);
			virtual void mouseEnter(const MouseEvent & event);
			virtual void mouseExit(const MouseEvent & event);

			cv::Mat original_image;

			const juce::Point<int> invalid_point;

			juce::Point<int> mouse_current_loc;
			juce::Point<int> mouse_previous_loc;
	};
}
