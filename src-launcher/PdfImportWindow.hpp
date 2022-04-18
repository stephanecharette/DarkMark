// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class PdfImportWindow : public DocumentWindow, public Button::Listener, public ThreadWithProgressWindow
	{
		public:

			PdfImportWindow(const std::string & dir, const VStr & v);

			virtual ~PdfImportWindow();

			virtual void resized() override;
			virtual void closeButtonPressed() override;
			virtual void userTriedToCloseWindow() override;
			virtual void buttonClicked(Button * button) override;

			virtual void run() override;

			const std::string base_directory;
			const VStr		filenames;

			Component canvas;

			Label			header_message;
			Label			txt_dpi;
			Slider			sl_dpi;
			ToggleButton	tb_do_not_resize;
			ToggleButton	tb_resize;
			TextEditor		ef_width;
			Label			txt_x;
			TextEditor		ef_height;
			ToggleButton	tb_keep_aspect_ratio;
			ToggleButton	tb_force_resize;
			ToggleButton	tb_save_as_png;
			ToggleButton	tb_save_as_jpeg;
			Label			txt_jpeg_quality;
			Slider			sl_jpeg_quality;
			TextButton		cancel;
			TextButton		ok;

			size_t			extra_lines_needed;
			size_t			number_of_imported_pages;
	};
}
