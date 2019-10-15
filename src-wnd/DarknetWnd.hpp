/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DarknetWnd : public DocumentWindow, public Button::Listener
	{
		public:

			DarknetWnd(DMContent & c);

			virtual ~DarknetWnd();

			virtual void closeButtonPressed();
			virtual void userTriedToCloseWindow();

			virtual void resized();

			virtual void buttonClicked(Button * button);

			void create_YOLO_configuration_files();
			void create_Darknet_files();

			Value v_darknet_dir;
			Value v_enable_yolov3_tiny;
			Value v_enable_yolov3_full;
			Value v_training_images_percentage;
			Value v_image_size;
			Value v_batch_size;
			Value v_subdivisions;
			Value v_iterations;
			Value v_enable_hue;
			Value v_enable_flip;

			DMContent & content;
			ProjectInfo & info;
			Component canvas;
			PropertyPanel pp;
			TextButton ok_button;
			TextButton cancel_button;
	};
}
