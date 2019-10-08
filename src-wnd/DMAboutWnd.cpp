/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMAboutCanvas::DMAboutCanvas()
{
	opacity_for_swirl	= 0.5;
	opacity_for_darknet	= 0.8;

	delta_for_swirl		= -0.01;
	delta_for_darknet	= +0.01;

	startTimer(100); // in milliseconds

	return;
}


void dm::DMAboutCanvas::paint(Graphics & g)
{
	const Image white_background = AboutLogoWhiteBackground();
	const int w = getWidth();
	const int h = getHeight() - 50; // bottom 50 reserved for the text
	const int x = (w - white_background.getWidth()) / 2;
	const int y = (h - white_background.getHeight()) / 2;

	g.setOpacity(1.0);
	g.fillAll(Colours::lightgrey);
	g.drawImageAt(AboutLogoWhiteBackground(), x, y, false);

	g.setOpacity(opacity_for_swirl);
	g.drawImageAt(AboutLogoRedSwirl(), x, y, false);

	g.setOpacity(opacity_for_darknet);
	g.drawImageAt(AboutLogoDarknet(), x, y, false);

	g.setOpacity(1.0);
	g.setColour(Colours::black);
	g.drawMultiLineText(
		"C Code Run's DarkMark, (C) 2019 Stephane Charette\n"
		"Application to mark up images for use with Darknet.\n"
		"See https://www.ccoderun.ca/ for details.", 0, h + 10, w, Justification::centred);

	return;
}


void dm::DMAboutCanvas::timerCallback()
{
	opacity_for_swirl	+= delta_for_swirl;
	opacity_for_darknet	+= delta_for_darknet;

	const float min_alpha = 0.1;
	const float max_alpha = 1.0;

	if (opacity_for_swirl	< min_alpha) { opacity_for_swirl	= min_alpha;	delta_for_swirl		= +0.01; }
	if (opacity_for_swirl	> max_alpha) { opacity_for_swirl	= max_alpha;	delta_for_swirl		= -0.01; }
	if (opacity_for_darknet	< min_alpha) { opacity_for_darknet	= min_alpha;	delta_for_darknet	= +0.01; }
	if (opacity_for_darknet	> max_alpha) { opacity_for_darknet	= max_alpha;	delta_for_darknet	= -0.01; }

	repaint();

	return;
}


dm::DMAboutWnd::DMAboutWnd() :
		DialogWindow("DarkMark by C Code Run", Colours::lightgrey, true)
{
	setResizable(false, false);
	setContentNonOwned(&canvas, true);

	auto r = dmapp().wnd->getBounds();
	r = r.withSizeKeepingCentre(400, 400 + 50);
	setBounds(r);

	setVisible(true);

	return;
}


dm::DMAboutWnd::~DMAboutWnd()
{
	return;
}


void dm::DMAboutWnd::closeButtonPressed()
{
	// close button

	dmapp().about_wnd.reset(nullptr);

	return;
}


void dm::DMAboutWnd::userTriedToCloseWindow()
{
	// ALT+F4

	dmapp().about_wnd.reset(nullptr);

	return;
}
