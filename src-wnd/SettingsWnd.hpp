// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class SettingsWnd : public DocumentWindow, public Button::Listener, public Value::Listener, public Timer
	{
		public:

			SettingsWnd(DMContent & c);

			virtual ~SettingsWnd();

			virtual void closeButtonPressed() override;
			virtual void userTriedToCloseWindow() override;
			virtual void resized() override;
			virtual void buttonClicked(Button * button) override;
			virtual void valueChanged(Value & value) override;
			virtual void timerCallback() override;

			Value v_darkhelp_threshold;
			Value v_darkhelp_hierchy_threshold;
			Value v_darkhelp_non_maximal_suppression_threshold;
			Value v_scrollfield_width;
			Value v_scrollfield_marker_size;
			Value v_show_mouse_pointer;
			Value v_image_tiling;

			DMContent & content;
			Component canvas;
			PropertyPanel pp;
			TextButton ok_button;
	};
}
