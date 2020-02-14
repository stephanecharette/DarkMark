/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

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

			void create_YOLO_configuration_files();
			void create_Darknet_files();

			Value v_darknet_dir;
			Value v_enable_yolov3_tiny;
			Value v_enable_yolov3_full;
			Value v_train_with_all_images;
			Value v_training_images_percentage;
			Value v_image_size;
			Value v_batch_size;
			Value v_subdivisions;
			Value v_iterations;
			Value v_saturation;
			Value v_exposure;
			Value v_hue;
			Value v_enable_flip;
			Value v_angle;
			Value v_mosaic;
			Value v_cutmix;
			Value v_mixup;
			Value v_keep_augmented_images;

			DMContent & content;
			ProjectInfo & info;
			Component canvas;
			PropertyPanel pp;
			TextButton help_button;
			TextButton ok_button;
			TextButton cancel_button;

			SliderPropertyComponent * percentage_slider;
	};
}
