/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
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


	#if (DARKMARK_ENABLE_OPENCV_CSRT_TRACKER > 0)
	/* Not enabled by default.  See CM_definitions.cmake for details.
	 * Unless you are specifically working on CSRT I suggest that this option remain disabled.
	 *
	 * Each OpenCV object tracker needs to also have a class index so we know which class to apply.
	 */
	struct ObjectTracker
	{
		size_t class_idx;
		cv::Ptr<cv::Tracker> tracker;
	};
	typedef std::vector<ObjectTracker> ObjectTrackers;
	#endif


	/** The content of the main DarkMark window.  This is where all the action happens.  The @p DMContent window
	 * is where the image is shown, where Darknet/DarkHelp is managed, where all the images are sorted, etc.
	 *
	 * There is a single instance of this class, and it is passed in by reference into a lot of other classes.
	 *
	 * @li The parent object is of type @ref DMWnd.
	 * @li Important children are @ref DMCanvas (where images are drawn) and @ref ScrollField.
	 * @li Also see @ref image_filenames and @ref project_info.
	 */
	class DMContent final : public Component
	{
		public:

			DMContent(const std::string & prefix);

			virtual ~DMContent();

			virtual void resized() override;

			virtual bool keyPressed(const KeyPress &key) override;

			void start_darknet();

			void rebuild_image_and_repaint();

			DMContent & set_class(const size_t class_idx);

			DMContent & set_sort_order(const ESort new_sort_order);

			DMContent & set_labels(const EToggle toggle);

			DMContent & toggle_shade_rectangles();

			DMContent & toggle_bold_labels();

			DMContent & toggle_show_predictions(const EToggle toggle);

			DMContent & toggle_show_marks();

			DMContent & toggle_show_processing_time();

			DMContent & load_image(const size_t new_idx, const bool full_load = true);

			DMContent & save_text();

			DMContent & save_json();

			size_t count_marks_in_json(File & f);

			bool load_text();

			bool load_json();

			DMContent & show_darknet_window();

			DMContent & delete_current_image();

			/** Attempt to copy the marks from the given image (or rather, from the corresponding .json file).
			 * @returns @p true if at least 1 mark was copied
			 * @returns @p false if there was no .json file or no marks.
			 */
			bool copy_marks_from_given_image(const std::string & fn);

			DMContent & copy_marks_from_next_image();

			DMContent & copy_marks_from_previous_image();

			DMContent & accept_current_mark();
			
			DMContent & accept_all_marks();

			DMContent & erase_all_marks();

			PopupMenu create_class_menu();

			PopupMenu create_popup_menu();

			DMContent & gather_statistics();

			DMContent & review_marks();

			DMContent & rotate_every_image();

			DMContent & reload_resave_every_image();

			DMContent & show_jump_wnd();

			DMContent & show_message(const std::string & msg);

			/** Save the current image with annotations as a screenshot.
			 * @param [in] full_size When set to @p true, this will save the image at 100% zoom.  When set to false, this will
			 * save the image at exactly the same zoom level as what is currently displayed on the screen.
			 * @param [in] fn If not blank, the screenshot will be saved to this file.  Should end in @p .png or @p .jpg.
			 */
			DMContent & save_screenshot(const bool full_size = true, const std::string & fn = "");

			/// The text prefix used to store keys in configuration.  This differs for every project.
			std::string cfg_prefix;

			DMCanvas canvas;

			ScrollField scrollfield;
			int scrollfield_width;

			VMarks marks;
			VStr names;

			/// Index into the @ref names vector so we can easily find the special case of "empty image".
			size_t empty_image_name_index;

			ESort sort_order;
			EToggle show_labels;

			/// Toggle in configuration that determines if darknet predictions are hidden, shown, or automatic (meaning shown only if there are no "real" marks).
			EToggle show_predictions;

			/// Boolean flag triggered by the user to decide if the image is empty and should still have a .json and .txt file generated.
			bool image_is_completely_empty;

			/// Boolean switch in configuration that determines if real marks are shown or hidden.
			bool show_marks;

			/** Not in configuration, but instead a runtime setting that is a combination of @ref show_marks and @ref marks not
			 * empty.  Used to indicate whether the user is interacting primarily with marks or predictions on the screen.
			 */
			bool marks_are_shown;

			/** Not in configuration, but instead a runtime setting that is a combination of @ref show_marks, @ref show_predictions,
			 * and whether or not there are predictions to show on the screen.
			 */
			bool predictions_are_shown;

			/// Not all objects stored in @ref marks are user-defined marks, some are predictions.  This counts the number of user-defined marks.
			size_t number_of_marks;

			/// Not all objects stored in @ref marks are predictions, some are user-defined marks.  This counts the number of predictions.
			size_t number_of_predictions;

			double alpha_blend_percentage;
			bool shade_rectangles;
			bool all_marks_are_bold;
			bool show_processing_time;
			bool need_to_save;
			int selected_mark;

			std::string darknet_image_processing_time;	///< How long it took darknet to make predictions for the current image.

			DarkHelp::VColours annotation_colours;

			/// The most recently used class now determines the colour to use for the crosshairs.
			Colour crosshair_colour;

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

			BubbleMessageComponent bubble_message;

			#if (DARKMARK_ENABLE_OPENCV_CSRT_TRACKER > 0)
			/// OpenCV object trackers (e.g., CSRT).
			ObjectTrackers object_trackers;
			#endif
	};
}
