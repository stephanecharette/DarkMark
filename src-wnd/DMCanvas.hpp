/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMCanvas : public Component
	{
		public:

			DMCanvas();

			virtual ~DMCanvas();

			virtual void paint(Graphics & g);

			virtual void mouseMove(const MouseEvent & event);
			virtual void mouseEnter(const MouseEvent & event);
			virtual void mouseExit(const MouseEvent & event);
			virtual void mouseDown(const MouseEvent & event);
			virtual void mouseUp(const MouseEvent & event);
			virtual void mouseDrag(const MouseEvent & event);
			virtual void mouseWheelMove(const MouseEvent & event, const MouseWheelDetails & wheel);

			cv::Mat current_image;
			Image current_img;

			std::list<cv::Point> click_points;

			const juce::Point<int> invalid_point;
			const juce::Rectangle<int> invalid_rectangle;

			juce::Point<int> mouse_down_point;
			juce::Point<int> mouse_current_loc;
			juce::Point<int> mouse_previous_loc;

			juce::Rectangle<int> mouse_drag_rectangle;

			double zoom;
	};
}
