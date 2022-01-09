// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	typedef std::map<double, double> MDbl;

	class DMJumpWnd : public DocumentWindow, public Slider::Listener, public Timer
	{
		public:

			DMJumpWnd(DMContent & c);

			virtual ~DMJumpWnd();

			virtual void closeButtonPressed() override;
			virtual void userTriedToCloseWindow() override;

			virtual void sliderValueChanged(Slider * slider) override;

			virtual void timerCallback() override;

			virtual void paintOverChildren(Graphics & g) override;

			Slider slider;

			DMContent & content;

			/// Remember the locations (percentages) where the image sets need to be drawn overtop the slider component.
			MDbl markers;

	};
}
