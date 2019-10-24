/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/// Different ways in which the images may be sorted.
	enum class ESort
	{
		kInvalid					= 0,
		kAlphabetical				,
		kTimestamp					,
		kCountMarks					,
		kRandom
	};

	class DMContent final : public Component
	{
		public:

			DMContent();

			virtual ~DMContent();

			virtual void resized();

			virtual bool keyPressed(const KeyPress &key);

			void start_darknet();

			void rebuild_image_and_repaint();

			DMContent & set_class(const size_t class_idx);

			DMContent & set_sort_order(const ESort new_sort_order);

			DMContent & set_labels(const EToggle toggle);

			DMContent & toggle_bold_labels();

			DMContent & toggle_show_predictions(const EToggle toggle);

			DMContent & toggle_show_processing_time();

			DMContent & load_image(const size_t new_idx);

			DMContent & save_text();

			DMContent & save_json();

			size_t count_marks_in_json(File & f);

			bool load_text();

			bool load_json();

			DMContent & show_darknet_window();

			DMContent & delete_current_image();

			DMContent & accept_all_marks();

			DMContent & erase_all_marks();

			PopupMenu create_class_menu();

			PopupMenu create_popup_menu();

			DMContent & gather_statistics();

			DMCanvas canvas;
			std::vector<DMCorner*> corners;

			VMarks marks;
			VStr names;

			ESort sort_order;
			EToggle show_labels;
			EToggle show_predictions;

			double alpha_blend_percentage;
			bool all_marks_are_bold;
			bool show_processing_time;
			bool need_to_save;
			int selected_mark;

			std::string darknet_image_processing_time;	///< How long it took darknet to make predictions for the current image.

			DarkHelp::VColours annotation_colours;

			cv::Mat original_image;
			cv::Mat scaled_image;

			/// The exact amount by which the image needs to be scaled.  @see @ref resized()
			double scale_factor;

			/// The final size to which the background image needs to be resized to fit perfectly into the canvas.  @see @ref resized()
			cv::Size scaled_image_size;

			cv::Size2d most_recent_size;
			size_t most_recent_class_idx;

			VStr image_filenames;
			size_t image_filename_index;

			std::string long_filename;
			std::string short_filename;
			std::string json_filename;
			std::string text_filename;

			ProjectInfo project_info;
	};
}
