// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DarknetWnd : public DocumentWindow, public Button::Listener, public Value::Listener
	{
		public:

			DarknetWnd(DMContent & c);

			virtual ~DarknetWnd();

			virtual void closeButtonPressed();
			virtual void userTriedToCloseWindow();

			virtual void resized();

			virtual bool keyPressed(const KeyPress & key);

			virtual void buttonClicked(Button * button);

			virtual void valueChanged(Value & value);

			void create_Darknet_training_and_validation_files(
					ThreadWithProgressWindow & progress_window,
					size_t & number_of_files_train			,
					size_t & number_of_files_valid			,
					size_t & number_of_annotated_images		,
					size_t & number_of_skipped_files		,
					size_t & number_of_marks				,
					size_t & number_of_empty_images			,
					size_t & number_of_dropped_empty_images	,
					size_t & number_of_resized_images		,
					size_t & number_of_images_not_resized	,
					size_t & number_of_tiles_created		,
					size_t & number_of_zooms_created		,
					size_t & number_of_dropped_annotations	);

			void find_all_annotated_images(ThreadWithProgressWindow & progress_window, VStr & annotated_images, VStr & skipped_images, size_t & number_of_marks, size_t & number_of_empty_images);

			void resize_images(ThreadWithProgressWindow & progress_window, const VStr & annotated_images, VStr & all_output_images, size_t & number_of_resized_images, size_t & number_of_images_not_resized, size_t & number_of_marks, size_t & number_of_empty_images);

			void tile_images(ThreadWithProgressWindow & progress_window, const VStr & annotated_images, VStr & all_output_images, size_t & number_of_marks, size_t & number_of_tiles_created, size_t & number_of_empty_images);

			void random_zoom_images(ThreadWithProgressWindow & progress_window, const VStr & annotated_images, VStr & all_output_images, size_t & number_of_marks, size_t & number_of_zooms_created, size_t & number_of_empty_images);

			void drop_small_annotations(ThreadWithProgressWindow & progress_window, const VStr & all_output_images, size_t & number_of_annotations_dropped);

			void create_Darknet_configuration_file(ThreadWithProgressWindow & progress_window);
			void create_Darknet_shell_scripts();

			CfgHandler cfg_handler;

			Value v_cfg_template;
			Value v_extra_flags;
			Value v_train_with_all_images;
			Value v_training_images_percentage;
			Value v_limit_validation_images;
			Value v_image_width;
			Value v_image_height;
			Value v_batch_size;
			Value v_subdivisions;
			Value v_iterations;
			Value v_learning_rate;
			Value v_max_chart_loss;
			Value v_image_type;
			Value v_do_not_resize_images;
			Value v_resize_images;
			Value v_tile_images;
			Value v_zoom_images;
			Value v_limit_negative_samples;
			Value v_remove_small_annotations;
			Value v_annotation_area_size;
			Value v_recalculate_anchors;
			Value v_anchor_clusters;
			Value v_class_imbalance;
			Value v_restart_training;
			Value v_delete_temp_weights;
			Value v_save_weights;
			Value v_saturation;
			Value v_exposure;
			Value v_hue;
			Value v_enable_flip;
			Value v_angle;
			Value v_mosaic;
			Value v_cutmix;
			Value v_mixup;
			Value v_verbose_output;
			Value v_keep_augmented_images;
			Value v_show_receptive_field;

			DMContent & content;
			ProjectInfo & info;
			Component canvas;
			PropertyPanel pp;
			TextButton help_button;
			DrawableButton youtube_button;
			TextButton ok_button;
			TextButton cancel_button;

			BubbleMessageComponent bubble;

			ButtonPropertyComponent * template_button;
			SliderPropertyComponent * percentage_slider;
			BooleanPropertyComponent * recalculate_anchors_toggle;
			BooleanPropertyComponent * class_imbalance_toggle;
			BooleanPropertyComponent * remove_small_annotations_toggle;
			SliderPropertyComponent * annotation_area_size_slider;

			/** Certain fields show a bubble with additional details when selected or deselected.
			 * This information is stored in this structure.
			 */
			struct BubbleInfo
			{
				public:

					PropertyComponent * component;

					// if the component is BooleanPropertyComponent, then:
					bool highlight_if_enabled;

					// if the component is SliderPropertyComponent, then:
					int highlight_if_less_than;
					int highlight_if_more_than;

					// the message to be shown when the conditions above have triggered
					std::string msg;

					BubbleInfo(BooleanPropertyComponent * c, const bool state, const std::string & text) :
						component(c),
						highlight_if_enabled(state),
						highlight_if_less_than(-1),
						highlight_if_more_than(-1),
						msg(text)
					{
						return;
					}

					BubbleInfo(SliderPropertyComponent * c, const int min, const int max, const std::string & text) :
						component(c),
						highlight_if_enabled(false),
						highlight_if_less_than(min),
						highlight_if_more_than(max),
						msg(text)
					{
						return;
					}
			};
			std::vector<BubbleInfo> highlight_conditions_and_messages;
	};
}
