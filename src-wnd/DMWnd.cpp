/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMWnd::DMWnd() :
	DocumentWindow(
	String("DarkMark v" DARKMARK_VERSION),
	Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
	DocumentWindow::TitleBarButtons::allButtons)
{
//	setContentNonOwned( &canvas, false );

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

#if 0
	if (cfg().containsKey("DMWnd"))
	{
		restoreWindowStateFromString( cfg().getValue("DMWnd") );
	}

	setFullScreen(cfg().kiosk_mode);
#endif

	setVisible(true);

	return;
}


dm::DMWnd::~DMWnd(void)
{
//	cfg().setValue("DMWnd", getWindowStateAsString());

	return;
}


void dm::DMWnd::closeButtonPressed(void)
{
	setVisible(false);
	JUCEApplication::getInstance()->systemRequestedQuit();

	return;
}


bool dm::DMWnd::keyPressed(const KeyPress &key)
{
	const ModifierKeys modifiers = key.getModifiers();
	if (key.isKeyCode(KeyPress::F1Key))
	{
		// ...todo
		return true; // true == consume the keystroke
	}
	else if (key.isKeyCode(KeyPress::F2Key) && modifiers.isShiftDown())
	{
//		File f = cfg().getFile();
//		f.revealToUser();
		return true; // true == consume the keystroke
	}
	else if (key.isKeyCode(KeyPress::F4Key) && modifiers.isShiftDown())
	{
//		File f( get_log_filename() );
//		f.startAsProcess();
		return true; // true == consume the keystroke
	}

	return false; // false == keystroke not handled
}
