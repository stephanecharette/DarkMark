/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMCorner::DMCorner() :
	invalid_point(-1, -1),
	mouse_current_loc(invalid_point),
	mouse_previous_loc(invalid_point)
{
	// why does this not work?
	setMouseCursor(MouseCursor::NoCursor);

	addMouseListener(this, false);

	return;
}


dm::DMCorner::~DMCorner()
{
	return;
}


void dm::DMCorner::resized()
{
	return;
}


void dm::DMCorner::paint(Graphics & g)
{
	g.fillAll(Colours::lightgreen);

	g.setColour(Colours::black);
	const int h = getHeight();
	const int w = getWidth();

	for (int x = 10; x < w; x += 11)
	{
		g.drawVerticalLine(x, 0, h);
	}

	for (int y = 10; y < h; y += 11)
	{
		g.drawHorizontalLine(y, 0, w);
	}

	if (original_image.empty() == false)
	{
		cv::Point p(642, 477);
		p -= cv::Point(3, 3);
		cv::Mat roi(original_image(cv::Rect(p.x, p.y, 16, 16)));
		for (int row_index = 0; row_index < 16; row_index ++)
		{
			const uint8_t * src_ptr = roi.ptr(row_index);
			for (int col_index = 0; col_index < 16; col_index ++)
			{
				const uint8_t & red		= src_ptr[col_index * 3 + 2];
				const uint8_t & green	= src_ptr[col_index * 3 + 1];
				const uint8_t & blue	= src_ptr[col_index * 3 + 0];
				g.setColour(Colour(red, green, blue));
				g.fillRect(col_index * 11, row_index * 11, 10, 10);
			}

		g.setColour(Colour(255, 0, 255));
		g.drawVerticalLine(45, 37, h);
		g.drawHorizontalLine(37, 45, w);
		g.fillRect(44, 34, 10, 10);
		}

	}

	if (mouse_current_loc != invalid_point)
	{
		g.setColour(Colours::red);
		g.drawHorizontalLine(	mouse_current_loc.y, 0, getWidth()	);
		g.drawVerticalLine(		mouse_current_loc.x, 0, getHeight()	);
	}

	return;
}


void dm::DMCorner::mouseMove(const MouseEvent & event)
{
	// record where the mouse is and invalidate the drawing
	mouse_previous_loc	= mouse_current_loc;
	mouse_current_loc	= event.getPosition();
	repaint();

	return;
}


void dm::DMCorner::mouseEnter(const MouseEvent & event)
{
	mouse_previous_loc	= invalid_point;
	mouse_current_loc	= event.getPosition();

	repaint();

	return;
}


void dm::DMCorner::mouseExit(const MouseEvent & event)
{
	mouse_previous_loc	= invalid_point;
	mouse_current_loc	= invalid_point;

	repaint();

	return;
}
