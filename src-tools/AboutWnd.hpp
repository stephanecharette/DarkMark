// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class AboutCanvas : public Component, public Timer
	{
		public:

			AboutCanvas();

			virtual void paint(Graphics & g);

			virtual void timerCallback();

			float opacity_for_swirl;
			float opacity_for_darknet;
			float delta_for_swirl;
			float delta_for_darknet;
	};

	class AboutWnd : public DialogWindow
	{
		public:

			AboutWnd();

			virtual ~AboutWnd();

			virtual void closeButtonPressed();
			virtual void userTriedToCloseWindow();

			AboutCanvas canvas;
	};
}
