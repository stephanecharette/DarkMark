/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMReviewWnd::DMReviewWnd(DMContent & c) :
	DocumentWindow("DarkMark v" DARKMARK_VERSION " Review", Colours::darkgrey, TitleBarButtons::closeButton),
	content(c)
{
	setContentNonOwned		(&notebook, true);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("ReviewWnd"))
	{
		restoreWindowStateFromString( cfg().getValue("ReviewWnd") );
	}
	else
	{
		centreWithSize(600, 200);
	}

	setVisible(true);

	return;
}


dm::DMReviewWnd::~DMReviewWnd()
{
	cfg().setValue("ReviewWnd", getWindowStateAsString());

	return;
}


void dm::DMReviewWnd::closeButtonPressed()
{
	// close button

	dmapp().review_wnd.reset(nullptr);

	return;
}


void dm::DMReviewWnd::userTriedToCloseWindow()
{
	// ALT+F4

	dmapp().review_wnd.reset(nullptr);

	return;
}


void dm::DMReviewWnd::rebuild_notebook()
{
	while (notebook.getNumTabs() > 0)
	{
		notebook.removeTab(0);
	}

	for (auto iter : m)
	{
		const size_t class_idx = iter.first;
		const MReviewInfo & mri = m.at(class_idx);

		std::string name = "#" + std::to_string(class_idx);
		if (content.names.size() > class_idx)
		{
			// if we can, we'd much rather use the "official" name for this class
			name = content.names.at(class_idx);
		}

		Log("creating a notebook tab for class \"" + name + "\", mri has " + std::to_string(mri.size()) + " entries");

		notebook.addTab(name, Colours::darkgrey, new DMReviewCanvas(mri), true);
	}

	return;
}
