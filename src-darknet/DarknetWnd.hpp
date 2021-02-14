// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

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

			virtual void buttonClicked(Button * button);

			virtual void valueChanged(Value & value);

			void create_Darknet_training_and_validation_files(
					ThreadWithProgressWindow & progress_window,
					size_t & number_of_files_train	,
					size_t & number_of_files_valid	,
					size_t & number_of_skipped_files,
					size_t & number_of_marks		,
					size_t & number_of_tiles		);

			void create_Darknet_training_and_validation_files_do_not_resize_images(
					ThreadWithProgressWindow & progress_window,
					VStr & images_to_use			,
					VStr & images_skipped			,
					size_t & number_of_files_train	,
					size_t & number_of_files_valid	,
					size_t & number_of_skipped_files,
					size_t & number_of_marks		);

			void resize_images(ThreadWithProgressWindow & progress_window, VStr & images_to_use);

			void tile_images(ThreadWithProgressWindow & progress_window, VStr & images_to_use, size_t & number_of_tiles);

			void create_Darknet_configuration_file();
			void create_Darknet_shell_scripts();

			CfgHandler cfg_handler;

			Value v_darknet_dir;
			Value v_cfg_template;
			Value v_train_with_all_images;
			Value v_training_images_percentage;
			Value v_image_width;
			Value v_image_height;
			Value v_batch_size;
			Value v_subdivisions;
			Value v_iterations;
			Value v_learning_rate;
			Value v_max_chart_loss;
			Value v_do_not_resize_images;
			Value v_resize_images;
			Value v_tile_images;
			Value v_recalculate_anchors;
			Value v_anchor_clusters;
			Value v_class_imbalance;
			Value v_restart_training;
			Value v_delete_temp_weights;
			Value v_saturation;
			Value v_exposure;
			Value v_hue;
			Value v_enable_flip;
			Value v_angle;
			Value v_mosaic;
			Value v_cutmix;
			Value v_mixup;
			Value v_keep_augmented_images;
			Value v_show_receptive_field;

			DMContent & content;
			ProjectInfo & info;
			Component canvas;
			PropertyPanel pp;
			TextButton help_button;
			TextButton ok_button;
			TextButton cancel_button;

			SliderPropertyComponent * percentage_slider;
			BooleanPropertyComponent * recalculate_anchors_toggle;
			BooleanPropertyComponent * class_imbalance_toggle;
	};
}
