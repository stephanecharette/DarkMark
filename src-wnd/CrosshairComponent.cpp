/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::CrosshairComponent::CrosshairComponent() :
	Component("CrosshairComponent"),
	mouse_drag_is_enabled(false),
	invalid_point(-1, -1),
	invalid_rectangle(-1, -1, 0, 0),
	mouse_current_loc(invalid_point),
	mouse_previous_loc(invalid_point),
	mouse_down_loc(invalid_point),
	mouse_drag_rectangle(invalid_rectangle),
	need_to_rebuild_cache_image(true)
{
	// why does this not work?
	setMouseCursor(MouseCursor::NoCursor);

	addMouseListener(this, false);

	return;
}


dm::CrosshairComponent::~CrosshairComponent()
{
	return;
}


void dm::CrosshairComponent::resized()
{
	need_to_rebuild_cache_image = true;
	repaint();

	return;
}


void dm::CrosshairComponent::paint(Graphics & g)
{
	if (need_to_rebuild_cache_image or cached_image.isNull())
	{
		try
		{
			rebuild_cache_image();
			need_to_rebuild_cache_image = false;
		}
		catch (const std::exception & e)
		{
			cached_image = juce::Image();
			need_to_rebuild_cache_image = true;
			Log("Exception caught rebuilding cache image for " + this->getName().toStdString() + ": " + e.what());
		}
	}

	g.setOpacity(1.0f);

	if (cached_image.isValid())
	{
		const int h = getHeight();
		const int w = getWidth();
		g.drawImage(cached_image, 0, 0, w, h, 0, 0, w, h);

		if (mouse_drag_is_enabled and mouse_drag_rectangle != invalid_rectangle)
		{
			// The problem is if the user is moving the mouse upwards, or to the left, then we have a negative width
			// and/or height.  If that is the case, we need to reverse the points because when given a negative size,
			// drawRect() doesn't draw anything.

			int x1 = mouse_drag_rectangle.getX();
			int x2 = mouse_drag_rectangle.getWidth() + x1;
			int y1 = mouse_drag_rectangle.getY();
			int y2 = mouse_drag_rectangle.getHeight() + y1;

			// make sure x1, x2, y1, and y2 represent the TL and BR corners so the width and height will be a positive number
			if (x1 > x2) { std::swap(x1, x2); }
			if (y1 > y2) { std::swap(y1, y2); }

			juce::Rectangle<int> r(x1, y1, x2 - x1, y2 - y1);
			g.setColour(Colours::red);
			g.drawRect(r, 1);
		}
		else if (mouse_current_loc != invalid_point)
		{
			paint_crosshairs(g);
		}
	}
	else
	{
		g.fillAll(Colours::lightgreen);
	}

	return;
}


void dm::CrosshairComponent::paint_crosshairs(Graphics & g)
{
	if (mouse_current_loc != invalid_point)
	{
		const int h = getHeight();
		const int w = getWidth();

		g.setOpacity(1.0f);
		g.setColour(Colours::red);
		g.drawHorizontalLine(	mouse_current_loc.y, 0, w);
		g.drawVerticalLine(		mouse_current_loc.x, 0, h);
	}

	return;
}


void dm::CrosshairComponent::mouseMove(const MouseEvent & event)
{
	// record where the mouse is and invalidate the drawing
	mouse_previous_loc	= mouse_current_loc;
	mouse_current_loc	= event.getPosition();

	if (mouse_previous_loc == invalid_point)
	{
		// repaint the entire window
		repaint();
	}
	else
	{
		const int w = getWidth();
		const int h = getHeight();

		// repaint only the old location and the new location
		repaint(mouse_previous_loc.x, 0, 1, h);
		repaint(0, mouse_previous_loc.y, w, 1);

		repaint(mouse_current_loc.x, 0, 1, h);
		repaint(0, mouse_current_loc.y, w, 1);
	}

	return;
}


void dm::CrosshairComponent::mouseEnter(const MouseEvent & event)
{
	mouse_previous_loc	= invalid_point;
	mouse_current_loc	= event.getPosition();

	// better to invalidate the entire window on mouseEnter() and mouseExit()
	// originally I tried to be smarter about the area to repaint, but that left artifacts on the screen
	repaint();

	return;
}


void dm::CrosshairComponent::mouseExit(const MouseEvent & event)
{
	mouse_previous_loc	= invalid_point;
	mouse_current_loc	= invalid_point;

	// better to invalidate the entire window on mouseEnter() and mouseExit()
	// originally I tried to be smarter about the area to repaint, but that left artifacts on the screen
	repaint();

	return;
}


void dm::CrosshairComponent::mouseDown(const MouseEvent & event)
{
	mouse_previous_loc	= mouse_current_loc;
	mouse_current_loc	= event.getPosition();

	mouse_drag_rectangle.setPosition(mouse_current_loc);
	mouse_drag_rectangle.setSize(0, 0);

	return;
}


void dm::CrosshairComponent::mouseUp(const MouseEvent & event)
{
	mouse_previous_loc		= mouse_current_loc;
	mouse_current_loc		= event.getPosition();

	if (mouse_drag_is_enabled and mouse_drag_rectangle != invalid_rectangle)
	{
		// re-orient the rectangle so TL is smaller than BR, otherwise we'll get a negative width/height which confuses things
		const int x1 = mouse_drag_rectangle.getTopLeft().x;
		const int y1 = mouse_drag_rectangle.getTopLeft().y;
		const int x2 = mouse_current_loc.x;
		const int y2 = mouse_current_loc.y;

		mouse_drag_rectangle.setLeft	(std::min(x1, x2));
		mouse_drag_rectangle.setTop		(std::min(y1, y2));
		mouse_drag_rectangle.setRight	(std::max(x1, x2));
		mouse_drag_rectangle.setBottom	(std::max(y1, y2));

		if (mouse_drag_rectangle.getWidth() > 2 and mouse_drag_rectangle.getHeight() > 2)
		{
			mouseDragFinished(mouse_drag_rectangle);
		}
	}
	mouse_drag_rectangle = invalid_rectangle;

	need_to_rebuild_cache_image = true;
	repaint();

	return;
}


void dm::CrosshairComponent::mouseDrag(const MouseEvent & event)
{
	mouse_previous_loc	= mouse_current_loc;
	mouse_current_loc	= event.getPosition();

	const auto p1 = mouse_drag_rectangle.getTopLeft();
	const auto p2 = mouse_current_loc;
	const auto w = p2.x - p1.x;
	const auto h = p2.y - p1.y;

	mouse_drag_rectangle.setWidth(w);
	mouse_drag_rectangle.setHeight(h);

	repaint();

	return;
}


#if 0
void dm::CrosshairComponent::mouseWheelMove(const MouseEvent & event, const MouseWheelDetails & wheel)
{
	if (wheel.deltaY < 0)
	{
		zoom_factor *= 1.1;
		need_to_rebuild_cache_image = true;
		repaint();
	}
	else if (wheel.deltaY > 0)
	{
		zoom_factor /= 1.1;
		need_to_rebuild_cache_image = true;
		repaint();
	}

	// limit zoom to some decent values
	if (zoom_factor > 6.7275000) { zoom_factor = 6.7275000; }
	if (zoom_factor < 0.0520987) { zoom_factor = 0.0520987; }

	std::cout << "zoom=" << zoom_factor << std::endl;

	return;
}
#endif
