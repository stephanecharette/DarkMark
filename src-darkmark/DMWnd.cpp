/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

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

	CrosshairComponent::crosshair_colour = Colour::fromString(cfg().get_str("crosshair_colour"));

	content.start_darknet();

	setVisible(true);

	return;
}


dm::DMWnd::~DMWnd(void)
{
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
