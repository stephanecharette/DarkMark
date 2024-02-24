// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

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
	// (this only makes sense when the images are sorted alphabetically)
	auto filenames = content.image_filenames;
	std::sort(filenames.begin(), filenames.end());
	filenames.push_back(" "); // insert a "dummy" file so when we process for markers we'll trigger and add a record for the last set of files

	File previous_dir;
	double files_in_section	= 0;
	double files_with_json	= 0;
	for (size_t idx = 0; idx < filenames.size(); idx ++)
	{
		const std::string & fn = filenames.at(idx);
		File file	= File(fn);
		File json	= file.withFileExtension(".json");
		File parent	= file.getParentDirectory();
		if (parent != previous_dir)
		{
			// new directory!  put a marker here and remember this location
			const double location = (double)idx / (double)filenames.size();
			const double percentage_with_json = (double)files_with_json / (double)files_in_section;
			markers[location] = percentage_with_json;
			previous_dir = parent;
			files_in_section = 0;
			files_with_json = 0;
		}

		files_in_section ++;
		files_with_json += (json.existsAsFile() ? 1 : 0);
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


void dm::DMJumpWnd::sliderValueChanged(Slider * changed_slider)
{
	if (changed_slider)
	{
		size_t idx = changed_slider->getValue() - 1;
		if (idx >= content.image_filenames.size())
		{
			// something is wrong with the index, pick a safe number to use instead
			idx = 0;
		}

		if (idx != content.image_filename_index)
		{
			// this just does a "quick" load of the image and skips both predictions and markup
			content.load_image(idx, false, true);

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
	if (content.sort_order == ESort::kAlphabetical)
	{
		g.setOpacity(0.25);

		const double width	= getWidth();
		const double height	= getHeight();
		double old_location	= 0.0;
		for (auto iter : markers)
		{
			const double location = iter.first;
			const double percentage_with_json = iter.second;
			const double invert_percentage = 1.0 - percentage_with_json;

			if (location > 0.0)
			{
				auto colour = Colours::white;
				if (percentage_with_json >= 0.99)
				{
					colour = Colours::green;
				}
				else if (percentage_with_json <= 0.01)
				{
					colour = Colours::red;
				}
				g.setColour(colour);

				const double new_location = location * width;
				g.drawVerticalLine(std::round(new_location), 0.0, height);
				g.drawHorizontalLine(std::round(invert_percentage * height), old_location, new_location);
				old_location = new_location;
			}
		}
	}

	return;
}
