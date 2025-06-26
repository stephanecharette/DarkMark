// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

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
		kRandom						,
		kSimilarMarks				,
		kMinimumIoU					,
		kAverageIoU					,
		kMaximumIoU					,
		kNumberOfPredictions		,
		kNumberOfDifferences		,
		kPredictionsWithoutAnnotations,
		kAnnotationsWithoutPredictions
	};


	/** The content of the main DarkMark window.  This is where all the action happens.  The @p DMContent window
	 * is where the image is shown, where Darknet/DarkHelp is managed, where all the images are sorted, etc.
	 *
	 * There is a single instance of this class, and it is passed in by reference into a lot of other classes.
	 *
	 * @li The parent object is of type @ref DMWnd.
	 * @li Important children are @ref DMCanvas (where images are drawn) and @ref ScrollField.
	 * @li Also see @ref image_filenames and @ref project_info.
	 */
	class DMContent final : public Component, public Timer
	{
		public:

			DMContent(const std::string & prefix);

			virtual ~DMContent();

			virtual void resized() override;

			virtual bool keyPressed(const KeyPress &key) override;

			void start_darknet();

			virtual void paintOverChildren(Graphics & g) override;

			void rebuild_image_and_repaint();

			DMContent & set_class(const size_t class_idx);

			DMContent & set_sort_order(const ESort new_sort_order);

			DMContent & set_labels(const EToggle toggle);

			DMContent & toggle_shade_rectangles();

			DMContent & toggle_bold_labels();

			DMContent & toggle_show_predictions(const EToggle toggle);

			DMContent & toggle_show_marks();

			DMContent & toggle_black_and_white_mode();

			DMContent & toggle_show_processing_time();

			DMContent & cycle_heatmaps();
			DMContent & toggle_heatmaps();

			DMContent & load_image(const size_t new_idx, const bool full_load = true, const bool display_immediately = false);

			DMContent & save_text();

			DMContent & save_json();

			DMContent & import_text_annotations(const VStr & image_filenames);

			size_t build_id_from_classes(File & f);

			size_t count_marks_in_json(File & f, const bool for_sorting_purposes=false);

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

			DMContent & review_iou();

			DMContent & zoom_and_review();

			DMContent & rotate_every_image();

			DMContent & flip_images();

			DMContent & delete_rotate_and_flip_images();

			DMContent & move_empty_images();

			DMContent & reload_resave_every_image();

			DMContent & show_jump_wnd();

			DMContent & show_message(const std::string & msg);

			DMContent & resize_tl_tr();

			/** Save the current image with annotations as a screenshot.
			 * @param [in] full_size When set to @p true, this will save the image at 100% zoom.  When set to false, this will
			 * save the image at exactly the same zoom level as what is currently displayed on the screen.
			 * @param [in] fn If not blank, the screenshot will be saved to this file.  Should end in @p .png or @p .jpg.
			 */
			DMContent & save_screenshot(const bool full_size = true, const std::string & fn = "");

			/** Draw something on the screen to highlight the given image area.  The rectangle given uses the image coordinates,
			 * not the screen coordinates.  So if the image is zoomed, the this function will figure out how much the rectangle
			 * also needs to be zoomed.
			 */
			DMContent & highlight_rectangle(const cv::Rect & r);

			virtual void timerCallback() override;

			DMContent & create_threshold_image();

			bool snap_annotation(int idx);

			/// The text prefix used to store keys in configuration.  This differs for every project.
			std::string cfg_prefix;

			bool show_window;

			DMCanvas canvas;

			ScrollField scrollfield;
			int scrollfield_width;

			float highlight_x;
			float highlight_y;
			float highlight_inside_size;
			float highlight_outside_size;

			VMarks marks;
			VStr names;

			/// Index into the @ref names vector so we can easily find the special case of "empty image".
			size_t empty_image_name_index;

			int tl_name_index;	///< Set to -1 by default, or to the index of the TL name if it exists.
			int tr_name_index;	///< Set to -1 by default, or to the index of the TR name if it exists.

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
			bool show_mouse_pointer;
			bool IoU_info_found;
			int corner_size;
			int selected_mark;

			std::string darknet_image_processing_time;	///< How long it took darknet to make predictions for the current image.

			DarkHelp::VColours annotation_colours;

			/// The most recently used class now determines the colour to use for the crosshairs.
			Colour crosshair_colour;

			std::atomic<bool> images_are_loading;

			cv::Mat original_image;
			cv::Mat scaled_image;
			cv::Mat heatmap_image;

			/// Image to which we've applied black-and-white thresholding, and converted back to a RGB image.
			/// This is only created when you press 'W' to enable black-and-white mode.
			cv::Mat black_and_white_image;

			/// Should we show colour or black-and-white (threshold) image?
			bool black_and_white_mode_enabled;

			int black_and_white_threshold_blocksize;
			double black_and_white_threshold_constant;
			int snap_horizontal_tolerance;
			int snap_vertical_tolerance;
			bool snapping_enabled;

			int dilate_erode_mode;	// 0=disabled, 1=dilate only, 2=dilate+erode, 3=erode+dilate, 4=erode only
			int dilate_kernel_size;
			int dilate_iterations;
			int erode_kernel_size;
			int erode_iterations;

			bool heatmap_enabled;
			double heatmap_alpha_blend;
			double heatmap_threshold;
			int heatmap_visualize;
			int heatmap_class_idx;

			/// The exact amount by which the image needs to be scaled.  @see @ref resized()
			double scale_factor; ///< @todo can this be removed now? replaced by current_zoom_factor?

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

			VStr images_without_json;

			double user_specified_zoom_factor;	///< Manual zoom override.  Should be between 0.1 and about 5.0.  Set to -1 to use "automatic" zoom that fills the screen.
			double previous_zoom_factor;		///< Previously-used zoom so we know what to restore when the user presses SPACEBAR,
			double current_zoom_factor;			///< Actual zoom value used to resize the image. @todo is this the same as @ref scale_factor

			/** Coordinate of interest when zooming into or out of an image.  Be careful, this can be invalid coordinates if the
			 * mouse is beyond the image boundaries when zoom is activated.
			 *
			 * @note The point is in the *original image* coordinate space (meaning unzoomed).
			 *
			 * @see @ref dm::CrosshairComponent::zoom_image_offset
			 */
			cv::Point zoom_point_of_interest;

			SId zoom_review_marks_remaining;

			/// Keep track of which classes are turned on in the DarkMark filters.  When set, this will impact the zoom-and-review behaviour.
			SId filter_use_this_subset_of_class_ids;

			// merge mode
			bool merge_mode_active = false;
			bool mass_delete_mode_active = false;
			std::vector<Mark> merge_start_marks; // Holds marks from the first key frame.
			size_t merge_start_index = 0; // Holds the index of the first key frame.
			void startMergeMode();
			void selectMergeKeyFrame(); // Called on mouse click in merge mode.
			void interpolateMarks(const std::vector<Mark> &startMarks,

								  const std::vector<Mark> &endMarks,

								  int numIntermediateFrames);
			cv::Rect2d convertToNormalized(const cv::Rect &areaInScreenCoords);
	};
}
