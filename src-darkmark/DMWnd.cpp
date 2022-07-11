// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMWnd::DMWnd(const std::string & prefix) :
	DocumentWindow(
		String("DarkMark v" DARKMARK_VERSION),
		Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
		DocumentWindow::TitleBarButtons::allButtons),
	show_window(true),
	content(prefix)
{
	setContentNonOwned(&content, false);

	centreWithSize			(640, 480	);
	setUsingNativeTitleBar	(true		);
	setResizable			(true, true	);
	setDropShadowEnabled	(true		);

	if (dmapp().cli_options.count("editor") and dmapp().cli_options.at("editor") == "gen-darknet")
	{
		show_window = false;
		content.show_window = false;
		setVisible(false);
		removeFromDesktop();
	}
	else
	{
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

		setVisible(true);
	}

	content.load_image(0, false, true);

	// This next line can take a while to run.  Loading the neural network is very slow.
	// May want to investigate putting this on a thread.
	content.start_darknet();

	// give the window some time to draw itself, and then we'll reload the first image including passing it through darkhelp
	startTimer(50); // milliseconds

	return;
}


dm::DMWnd::~DMWnd(void)
{
	stopTimer();

	if (show_window)
	{
		cfg().setValue("DMWnd", getWindowStateAsString());
	}

	dmapp().jump_wnd	.reset(nullptr);
	dmapp().review_wnd	.reset(nullptr);
	dmapp().stats_wnd	.reset(nullptr);
	dmapp().darknet_wnd	.reset(nullptr);
//	dmapp().darkhelp_nn	.reset(nullptr);
	dmapp().settings_wnd.reset(nullptr);

	return;
}


void dm::DMWnd::closeButtonPressed(void)
{
	setVisible(false);
	dmapp().wnd.reset(nullptr);

	if (dmapp().cli_options.count("project_key"))
	{
		// if we were told to load a specific project, then don't restart the launcher
		dmapp().systemRequestedQuit();
	}
	else
	{
		dmapp().startup_wnd.reset(new StartupWnd);
	}

	return;
}


void dm::DMWnd::timerCallback()
{
	stopTimer();

	const auto & action = dmapp().cli_options["editor"];
	if (action == "gen-darknet")
	{
		setVisible(false);
		setBounds(0, 0, 0, 0);
		setMinimised(true);
		content.show_darknet_window();
	}
	else
	{
		content.load_image(0);
	}

	if (content.images_without_json.empty() == false)
	{
		String msg = "1 image file was found with \".txt\" annotations.";
		if (content.images_without_json.size() > 1)
		{
			msg = String(content.images_without_json.size()) + " image files were found with \".txt\" annotations.";
		}
		Log(msg.toStdString());

		content.import_text_annotations(content.images_without_json);
	}

	return;
}
