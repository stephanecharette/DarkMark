/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
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

			virtual void resized();

			virtual void rebuild_cache_image();

			DMCanvas & redraw_layers();

			/// Link to the parent which manages the content, including all the marks.
			DMContent & content;

			cv::Mat original_image;
			cv::Mat scaled_image;
			cv::Mat layer_background_image;
			cv::Mat layer_darkhelp;
			cv::Mat layer_class_names;
			cv::Mat layer_bounding_boxes;
			cv::Mat layer_points_of_interest;
			cv::Mat composited_image;

			std::list<cv::Point> click_points;
	};
}
