// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include <random>


namespace
{
	float get_random_delta()
	{
		static auto & e = dm::get_random_engine();
		static std::uniform_real_distribution<float> d(0.008f, 0.030f);

		return d(e);
	}
}


dm::AboutCanvas::AboutCanvas()
{
	opacity_for_swirl	= 1.0f;
	opacity_for_darknet	= 0.1f;
	delta_for_swirl		= 0.0f;
	delta_for_darknet	= 0.0f;

	// with JUCE 7, seems as if every once in a while the window is resized to the
	// canvas instead of the window telling the canvas how much room it has to draw,
	// so make sure we give a decent size to the canvas
	setSize(400, 450);

	startTimer(100); // in milliseconds

	return;
}


dm::AboutCanvas::~AboutCanvas()
{
	stopTimer();
}


void dm::AboutCanvas::paint(Graphics & g)
{
	const Image white_background = AboutLogoWhiteBackground();
	const int w = getWidth();
	const int h = getHeight() - 50; // bottom 50 reserved for the text
	const int x = (w - white_background.getWidth()) / 2;
	const int y = (h - white_background.getHeight()) / 2;

	g.setOpacity(1.0);
	g.fillAll(Colours::lightgrey);
	g.drawImageAt(AboutLogoWhiteBackground(), x, y, false);

	if (opacity_for_swirl	<= 0.0f or opacity_for_swirl	> 1.0f or
		opacity_for_darknet	<= 0.0f or opacity_for_darknet	> 1.0f )
	{
		dm::Log("something is wrong with the image opacity: "
			"swirl=" + std::to_string(opacity_for_swirl) +
			"darknet=" + std::to_string(opacity_for_darknet));
		opacity_for_swirl = 0.8f;
		opacity_for_darknet = 0.2f;
	}

	g.setOpacity(opacity_for_swirl);
	g.drawImageAt(AboutLogoRedSwirl(), x, y, false);

	g.setOpacity(opacity_for_darknet);
	g.drawImageAt(AboutLogoDarknet(), x, y, false);

	g.setOpacity(1.0);
	g.setColour(Colours::black);
	g.drawMultiLineText(
		"C Code Run's DarkMark (C) 2019-2025 Stephane Charette\n"
		"Manage your Darknet/YOLO projects with ease!\n"
		"See https://www.ccoderun.ca/darkmark/ for details.", 0, h + 10, w, Justification::centred);

	return;
}


void dm::AboutCanvas::timerCallback()
{
	opacity_for_swirl	+= delta_for_swirl;
	opacity_for_darknet	+= delta_for_darknet;

	const float min_alpha = 0.05f;
	const float max_alpha = 1.00f;

	if (opacity_for_swirl	< min_alpha) { opacity_for_swirl	= min_alpha;	delta_for_swirl		= 0.0f; }
	if (opacity_for_swirl	> max_alpha) { opacity_for_swirl	= max_alpha;	delta_for_swirl		= 0.0f; }
	if (opacity_for_darknet	< min_alpha) { opacity_for_darknet	= min_alpha;	delta_for_darknet	= 0.0f; }
	if (opacity_for_darknet	> max_alpha) { opacity_for_darknet	= max_alpha;	delta_for_darknet	= 0.0f; }

	if (std::abs(delta_for_swirl) < 0.008f)
	{
		if (opacity_for_swirl == max_alpha)
		{
			delta_for_swirl = 0.0f - get_random_delta();
		}
		else
		{
			delta_for_swirl = 0.0f + get_random_delta();
		}
	}

	if (std::abs(delta_for_darknet) < 0.008f)
	{
		if (opacity_for_darknet == max_alpha)
		{
			delta_for_darknet = 0.0f - get_random_delta();
		}
		else
		{
			delta_for_darknet = 0.0f + get_random_delta();
		}
	}

	repaint();

	return;
}


dm::AboutWnd::AboutWnd() :
		DialogWindow("DarkMark v" DARKMARK_VERSION " by C Code Run", Colours::lightgrey, true)
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(false, false	);
	setDropShadowEnabled	(true			);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	centreWithSize(400, 400 + 50);	// logo needs 400x400, but we also want to print a few lines of text at the bottom
	setVisible(true);

	return;
}


dm::AboutWnd::~AboutWnd()
{
	return;
}


bool dm::AboutWnd::keyPressed(const KeyPress & key)
{
	if (key.getKeyCode() == KeyPress::escapeKey)
	{
		dmapp().about_wnd.reset(nullptr);
		return true; // key has been handled
	}

	return false;
}


void dm::AboutWnd::closeButtonPressed()
{
	// close button
	dmapp().about_wnd.reset(nullptr);

	return;
}


void dm::AboutWnd::userTriedToCloseWindow()
{
	// ALT+F4
	dmapp().about_wnd.reset(nullptr);

	return;
}
