/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMCorner final : public CrosshairComponent
	{
		public:

			DMCorner(DMContent & c, const ECorner type);

			virtual ~DMCorner();

			virtual void mouseDown(const MouseEvent & event);

			virtual void rebuild_cache_image();

			virtual void set_class(size_t class_id);

			/// Link to the parent which manages the content, including all the marks.
			DMContent & content;

			ECorner corner;

			cv::Mat original_image;

			/// Size of each cell within the corner image.  Size does @em not include the right and bottom border.
			int cell_size;
			int cols;
			int rows;

			cv::Point top_left_point;
//			cv::Point offset;
//			cv::Point poi;
	};
}
