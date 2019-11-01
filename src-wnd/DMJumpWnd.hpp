/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	typedef std::set<double> SDbl;

	class DMJumpWnd : public DocumentWindow, public Slider::Listener, public Timer
	{
		public:

			DMJumpWnd(DMContent & c);

			virtual ~DMJumpWnd();

			virtual void closeButtonPressed();
			virtual void userTriedToCloseWindow();

			virtual void sliderValueChanged(Slider * slider);

			virtual void timerCallback();

			virtual void paintOverChildren(Graphics & g);

			Slider slider;

			DMContent & content;

			/// Remember the locations (percentages) where the image sets need to be drawn overtop the slider component.
			SDbl markers;

	};
}
