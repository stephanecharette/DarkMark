// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentRotateImages : public DocumentWindow, public Button::Listener, public ThreadWithProgressWindow
	{
		public:

			DMContentRotateImages(dm::DMContent & c);

			virtual ~DMContentRotateImages();

			virtual void resized() override;
			virtual void closeButtonPressed() override;
			virtual void userTriedToCloseWindow() override;
			virtual void buttonClicked(Button * button) override;

			virtual void run() override;

			DMContent & content;

			Component canvas;

			Label			header_message;

			ToggleButton	tb_090_degrees;
			ToggleButton	tb_180_degrees;
			ToggleButton	tb_270_degrees;

			ToggleButton	tb_save_as_png;
			ToggleButton	tb_save_as_jpeg;
			Label			txt_jpeg_quality;
			Slider			sl_jpeg_quality;

			ToggleButton	tb_annotated_images;
			ToggleButton	tb_empty_images;
			ToggleButton	tb_other_images;

			TextButton		cancel;
			TextButton		ok;

			size_t			images_created;
			size_t			images_skipped;
			size_t			images_already_exist;
			size_t			images_with_errors;
	};
}
