// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"

/** When set, this next define causes DarkMark to draw a square with some details
 * on the mouse, the state of zoom, and the currently selected mark (annotation).
 */
#define INCLUDE_DEBUG_MOUSE_LOC 0
#define DEBUG_MOUSE_LOC_RECT 600, 10, 250, 125


dm::CrosshairComponent::CrosshairComponent(DMContent & c) :
	Component("CrosshairComponent"),
	content(c),
	mouse_drag_is_enabled(false),
	mouse_drag_offset(0, 0),
	invalid_point(-1, -1),
	invalid_rectangle(-1, -1, 0, 0),
	mouse_current_loc(invalid_point),
	mouse_previous_loc(invalid_point),
	mouse_down_loc(invalid_point),
	mouse_drag_rectangle(invalid_rectangle),
	need_to_rebuild_cache_image(true),
	zoom_image_offset(0, 0)
{
	setBufferedToImage(false);

//	setRepaintsOnMouseActivity(true);

	return;
}


dm::CrosshairComponent::~CrosshairComponent()
{
	return;
}


MouseCursor dm::CrosshairComponent::getMouseCursor()
{
	if (content.show_mouse_pointer == false)
	{
		return MouseCursor::NoCursor;
	}

	return Component::getMouseCursor();
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
			Log("Exception caught rebuilding cache image for " + this->getName().toStdString() + " (" + content.long_filename + "): " + e.what());
		}
	}

	g.setOpacity(1.0f);

	if (cached_image.isValid())
	{
		const int h = getHeight();
		const int w = getWidth();
		g.drawImage(cached_image, 0, 0, w, h, zoom_image_offset.x, zoom_image_offset.y, w, h);

		if (mouse_drag_is_enabled and mouse_drag_rectangle != invalid_rectangle and content.canvas.is_panning == false)
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

			g.setColour(content.crosshair_colour);
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
		g.setColour(content.crosshair_colour);
		g.drawHorizontalLine(	mouse_current_loc.y, 0, w);
		g.drawVerticalLine(		mouse_current_loc.x, 0, h);
	}

	#if INCLUDE_DEBUG_MOUSE_LOC
	String str =
		"zoom offset:"
		" x=" + String(zoom_image_offset.x) +
		" y=" + String(zoom_image_offset.y) +
		"\n" +
		"zoom factor=" + String(content.current_zoom_factor) +
		"\n" +
		"wnd mouse loc:"
		" x=" + String(mouse_current_loc.x) +
		" y=" + String(mouse_current_loc.y) +
		"\n" +
		"img mouse loc:" +
		" x=" + String(std::round((mouse_current_loc.x + zoom_image_offset.x) / content.current_zoom_factor)) + "," +
		" y=" + String(std::round((mouse_current_loc.y + zoom_image_offset.y) / content.current_zoom_factor)) +
		"\n" +
		"scaled image:"
		" w=" + String(content.scaled_image_size.width) +
		" h=" + String(content.scaled_image_size.height) +
		"\n" +
		"current mark=" + String(content.selected_mark);

	if (content.selected_mark >= 0)
	{
		auto & m = content.marks.at(content.selected_mark);
		auto r = m.get_bounding_rect(content.scaled_image_size);
		str += "\n"
			"mark[" + String(content.selected_mark) + "]:"
			" x=" + String(r.x) +
			" y=" + String(r.y) +
			" w=" + String(r.width) +
			" h=" + String(r.height);
	}

	g.setColour(Colours::white);
	g.fillRect(DEBUG_MOUSE_LOC_RECT);
	g.setColour(Colours::blue);
	g.drawRect(DEBUG_MOUSE_LOC_RECT);
	g.drawFittedText(str, DEBUG_MOUSE_LOC_RECT, Justification::centred, 7);
	#endif

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

		#if INCLUDE_DEBUG_MOUSE_LOC
		repaint(DEBUG_MOUSE_LOC_RECT);
		#endif
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
	mouse_drag_offset	= cv::Point(0, 0);

	mouse_drag_rectangle.setPosition(mouse_current_loc);
	mouse_drag_rectangle.setSize(0, 0);

	return;
}


void dm::CrosshairComponent::mouseUp(const MouseEvent & event)
{
	mouse_previous_loc	= mouse_current_loc;
	mouse_current_loc	= event.getPosition() + juce::Point<int>(mouse_drag_offset.x, mouse_drag_offset.y);

	if (mouse_drag_is_enabled and mouse_drag_rectangle != invalid_rectangle)
	{
		// re-orient the rectangle so TL is smaller than BR, otherwise we'll get a negative width/height which confuses things
		const int x1 = mouse_drag_rectangle.getX();
		const int y1 = mouse_drag_rectangle.getY();
		const int x2 = mouse_current_loc.x;
		const int y2 = mouse_current_loc.y;

		mouse_drag_rectangle.setLeft	(std::min(x1, x2));
		mouse_drag_rectangle.setTop		(std::min(y1, y2));
		mouse_drag_rectangle.setRight	(std::max(x1, x2));
		mouse_drag_rectangle.setBottom	(std::max(y1, y2));

		if (mouse_drag_rectangle.getWidth() > 2 and mouse_drag_rectangle.getHeight() > 2)
		{
			mouseDragFinished(mouse_drag_rectangle, event);
		}
	}

	mouse_drag_rectangle		= invalid_rectangle;
	mouse_drag_offset			= cv::Point(0, 0);
	need_to_rebuild_cache_image	= true;

	repaint();

}


void dm::CrosshairComponent::mouseDrag(const MouseEvent & event)
{
	mouse_previous_loc = mouse_current_loc;
	mouse_current_loc = event.getPosition() + juce::Point<int>(mouse_drag_offset.x, mouse_drag_offset.y);

	if (mouse_drag_rectangle == invalid_rectangle or
		mouse_drag_rectangle.getX() == -1 or
		mouse_drag_rectangle.getY() == -1 or
		(mouse_drag_rectangle.getX() >= 0 and mouse_drag_rectangle.getY() >= 0 and
			(mouse_drag_rectangle.getWidth() == 0 or mouse_drag_rectangle.getHeight() == 0)))
	{
		// looks like this is the start of a new mouse drag
		need_to_rebuild_cache_image = true;
	}

	const auto p1 = mouse_drag_rectangle.getTopLeft();
	const auto p2 = mouse_current_loc;
	const auto w = p2.x - p1.x;
	const auto h = p2.y - p1.y;

	mouse_drag_rectangle.setWidth(w);
	mouse_drag_rectangle.setHeight(h);

	repaint();

	return;
}


void dm::CrosshairComponent::mouseWheelMove(const MouseEvent & event, const MouseWheelDetails & wheel)
{
	#if 0
	Log("DX="			+ std::to_string(wheel.deltaX)		+
		", DY="			+ std::to_string(wheel.deltaY)		+
		", inertial="	+ std::to_string(wheel.isInertial)	+
		", reversed="	+ std::to_string(wheel.isReversed)	+
		", smooth="		+ std::to_string(wheel.isSmooth)	);
	#endif

	if (content.user_specified_zoom_factor > 0.0)
	{
		// cancel the zoom instead of changing the image index
		content.keyPressed(KeyPress::createFromDescription("spacebar"));
		return;
	}

	const int idx = content.image_filename_index;

	// Not obvious:
	//
	// - When deltaY is negative, that means the wheel is being scrolled DOWN, so we need to INCREASE the image index.
	// - When deltaY is positive, that means the wheel is being scrolled UP, meaning we DECREASE the image index (but not past zero).
	//
	int change = 0;
	if (wheel.deltaY < 0.0f)
	{
		change = 1;
	}
	else if (wheel.deltaY > 0.0f and idx > 0)
	{
		change = -1;
	}

	if (change != 0)
	{
		content.load_image(idx + change, false, true);

		startTimer(500); // request a callback -- in milliseconds -- at which point in time we'll fully load this image
	}

	Component::mouseWheelMove(event, wheel);

	return;
}


void dm::CrosshairComponent::timerCallback()
{
	// if we get called, then we've been sitting on the same image for some time so go ahead and load the full image including all the marks
	stopTimer();

	content.load_image(content.image_filename_index);

	return;
}
