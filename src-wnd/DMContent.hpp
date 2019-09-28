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

			DMCanvas canvas;
			std::vector<DMCorner*> corners;

			VMarks marks;
			VStr names;

			int selected_mark;

			DarkHelp::VColours annotation_colours;
	};
}
