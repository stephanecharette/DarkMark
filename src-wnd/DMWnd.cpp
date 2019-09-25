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

//	setFullScreen(cfg().kiosk_mode);

	Log("loading darknet neural network");
	dmapp().darkhelp.reset(new DarkHelp(cfg().get_str("darknet_config"), cfg().get_str("darknet_weights"), cfg().get_str("darknet_names")));
	Log("neural network loaded in " + darkhelp().duration_string());

	content.canvas.rebuild_cache_image();
	for (size_t idx = 0; idx < 4; idx ++)
	{
		content.corner[idx].original_image = content.canvas.original_image;
		content.corner[idx].need_to_rebuild_cache_image = true;
	}

	setVisible(true);

	return;
}


dm::DMWnd::~DMWnd(void)
{
	cfg().setValue("DMWnd", getWindowStateAsString());

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
