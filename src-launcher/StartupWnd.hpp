// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class StartupWnd : public DocumentWindow, public Button::Listener
	{
		public:

			StartupWnd();

			virtual ~StartupWnd();

			virtual void closeButtonPressed();
			virtual void userTriedToCloseWindow();

			virtual void resized();

			virtual void buttonClicked(Button * button);

			virtual bool keyPressed(const KeyPress & key);

			virtual void updateButtons();

			void stop_refreshing_all_notebook_tabs();

			Component canvas;
			Notebook notebook;
			TextButton add_button;
			TextButton delete_button;
			TextButton import_video_button;
			TextButton import_pdf_button;
			TextButton open_folder_button;
			TextButton class_id_button;
			TextButton refresh_button;
			TextButton ok_button;
			TextButton cancel_button;
	};
}
