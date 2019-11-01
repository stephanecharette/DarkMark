/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMJumpWnd::DMJumpWnd(DMContent & c) :
	DocumentWindow("DarkMark v" DARKMARK_VERSION " Jump", Colours::darkgrey, TitleBarButtons::closeButton),
	slider(Slider::SliderStyle::LinearBar /* LinearHorizontal */, Slider::TextEntryBoxPosition::TextBoxBelow),
	content(c)
{
	setContentNonOwned		(&slider, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);
	setAlwaysOnTop			(true			);

	slider.setWantsKeyboardFocus(true);
	slider.grabKeyboardFocus();
	slider.setDoubleClickReturnValue(true, content.image_filename_index);
	slider.setNumDecimalPlacesToDisplay(0);
	slider.setPopupDisplayEnabled(false, false, nullptr);
	slider.setPopupMenuEnabled(false);
	slider.setRange(1.0, content.image_filenames.size(), 1.0);
	slider.setScrollWheelEnabled(true);
	slider.setSliderSnapsToMousePosition(true);
	slider.setTextBoxIsEditable(false);
	slider.setTextValueSuffix("/" + String(content.image_filenames.size()));
	slider.setValue(content.image_filename_index + 1.0);
	slider.addListener(this);

	// determine all the positions where we need to draw a marker (indicating different image sets)
	// (this only makes sense if the images are sorted alphabetically)
	if (content.sort_order == ESort::kAlphabetical)
	{
		File previous_dir;
		for (size_t idx = 0; idx < content.image_filenames.size(); idx ++)
		{
			const std::string & fn = content.image_filenames.at(idx);
			File dir = File(fn).getParentDirectory();
			if (dir != previous_dir)
			{
				// new directory!  remember this location
				markers.insert((double)idx / (double)content.image_filenames.size());
				previous_dir = dir;
			}
		}
	}

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("JumpWnd"))
	{
		restoreWindowStateFromString(cfg().getValue("JumpWnd"));
	}
	else
	{
		centreWithSize(600, 100);
	}

	setVisible(true);

	return;
}


dm::DMJumpWnd::~DMJumpWnd()
{
	cfg().setValue("JumpWnd", getWindowStateAsString());

	return;
}


void dm::DMJumpWnd::closeButtonPressed()
{
	// close button

	dmapp().jump_wnd.reset(nullptr);

	return;
}


void dm::DMJumpWnd::userTriedToCloseWindow()
{
	// ALT+F4

	dmapp().jump_wnd.reset(nullptr);

	return;
}


void dm::DMJumpWnd::sliderValueChanged(Slider * slider)
{
	if (slider)
	{
		size_t idx = slider->getValue() - 1;
		if (idx >= content.image_filenames.size())
		{
			// something is wrong with the index, pick a safe number to use instead
			idx = 0;
		}

		if (idx != content.image_filename_index)
		{
			// this just does a "quick" load of the image and skips both predictions and markup
			content.load_image(idx, false);

			startTimer(500); // request a callback -- in milliseconds -- at which point in time we'll fully load this image
		}
	}

	return;
}


void dm::DMJumpWnd::timerCallback()
{
	// if we get called, then we've been sitting on the same image for some time so go ahead and load the full image including all the marks
	stopTimer();

	content.load_image(content.image_filename_index);

	return;
}


void dm::DMJumpWnd::paintOverChildren(Graphics & g)
{
	if (content.sort_order == ESort::kAlphabetical and markers.empty() == false)
	{
		g.setColour(Colours::white);
		g.setOpacity(0.25);

		const double width		= getWidth();
		const double height		= getHeight();
		for (const double marker : markers)
		{
			if (marker > 0.0)
			{
				const double pos = marker * width;
				g.drawVerticalLine(pos, 0.0, height);
			}
		}
	}

	return;
}
