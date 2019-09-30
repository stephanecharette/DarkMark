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

			virtual void start_darknet();

			virtual void rebuild_image_and_repaint();

			virtual bool keyPressed(const KeyPress &key);

			DMCanvas canvas;
			std::vector<DMCorner*> corners;

			VMarks marks;
			VStr names;

			int selected_mark;

			DarkHelp::VColours annotation_colours;

			cv::Mat original_image;
			cv::Mat scaled_image;

			/// The final size to which the background image needs to be resized to fit perfectly into the canvas.  @see @ref resized()
			cv::Size scaled_image_size;

			/// The exact amount by which the image needs to be scaled.  @see @ref resized()
			double scale_factor;
	};
}
