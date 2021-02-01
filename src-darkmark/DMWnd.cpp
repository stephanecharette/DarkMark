// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMWnd::DMWnd(const std::string & prefix) :
	DocumentWindow(
		String("DarkMark v" DARKMARK_VERSION),
		Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
		DocumentWindow::TitleBarButtons::allButtons),
	content(prefix)
{
	setContentNonOwned(&content, false);

	centreWithSize			(640, 480	);
	setUsingNativeTitleBar	(true		);
	setResizable			(true, true	);
	setDropShadowEnabled	(true		);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("DMWnd"))
	{
		restoreWindowStateFromString( cfg().getValue("DMWnd") );
	}

	// This next line can take a while to run.  Loading the neural network is very slow.
	// May want to investigate putting this on a thread.
	content.start_darknet();

	content.load_image(0, false);

	setVisible(true);

	// give the window some time to draw itself, and then we'll reload the first image including passing it through darkhelp
	startTimer(50); // milliseconds

	return;
}


dm::DMWnd::~DMWnd(void)
{
	stopTimer();

	cfg().setValue("DMWnd", getWindowStateAsString());

	dmapp().jump_wnd	.reset(nullptr);
	dmapp().review_wnd	.reset(nullptr);
	dmapp().stats_wnd	.reset(nullptr);
	dmapp().darknet_wnd	.reset(nullptr);
	dmapp().darkhelp	.reset(nullptr);
	dmapp().settings_wnd.reset(nullptr);

	return;
}


void dm::DMWnd::closeButtonPressed(void)
{
	setVisible(false);
	dmapp().startup_wnd.reset(new StartupWnd);
	dmapp().wnd.reset(nullptr);

	return;
}


void dm::DMWnd::timerCallback()
{
	stopTimer();

	content.load_image(0);

	return;
}
