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

			virtual DMContent & load_image(const size_t new_idx);

			virtual DMContent & save_text();

			virtual DMContent & save_json();

			virtual size_t count_marks_in_json(File & f);

			virtual bool load_text();

			virtual bool load_json();

			DMCanvas canvas;
			std::vector<DMCorner*> corners;

			VMarks marks;
			VStr names;

			bool need_to_save;
			int selected_mark;

			DarkHelp::VColours annotation_colours;

			cv::Mat original_image;
			cv::Mat scaled_image;

			/// The final size to which the background image needs to be resized to fit perfectly into the canvas.  @see @ref resized()
			cv::Size scaled_image_size;

			/// The exact amount by which the image needs to be scaled.  @see @ref resized()
			double scale_factor;

			File image_directory;
			VStr image_filenames;
			size_t image_filename_index;
			std::string long_filename;
			std::string short_filename;
			std::string json_filename;
			std::string text_filename;
	};
}
