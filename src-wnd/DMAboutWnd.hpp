/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMAboutCanvas : public Component, public Timer
	{
		public:

			DMAboutCanvas();

			virtual void paint(Graphics & g);

			virtual void timerCallback();

			float opacity_for_swirl;
			float opacity_for_darknet;
			float delta_for_swirl;
			float delta_for_darknet;
	};

	class DMAboutWnd : public DialogWindow
	{
		public:

			DMAboutWnd();

			virtual ~DMAboutWnd();

			virtual void closeButtonPressed();
			virtual void userTriedToCloseWindow();

			DMAboutCanvas canvas;
	};
}
