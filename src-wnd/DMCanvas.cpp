/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


Image convert_opencv_mat_to_juce_image(cv::Mat mat)
{
	// if the image has 1 channel we'll convert it to greyscale RGB
	// if the image has 3 channels (RGB) then all is good,
	// (anything else will cause this function to throw)
	cv::Mat source;
	switch (mat.channels())
	{
		case 1:
		{
			cv::cvtColor(mat, source, cv::COLOR_GRAY2BGR);
			break;
		}
		case 3:
		{
			source = mat;	// nothing to do, use the image as-is
			break;
		}
		default:
		{
			throw std::logic_error("cv::Mat has an unexpected number of channels (" + std::to_string(mat.channels()) + ")");
		}
	}

	// create a JUCE Image the exact same size as the OpenCV mat we're using as our source
	Image image(Image::RGB, source.cols, source.rows, false);

	// iterate through each row of the image, copying the entire row as a series of consecutive bytes
	const size_t number_of_bytes_to_copy = 3 * source.cols; // times 3 since each pixel contains 3 bytes (RGB)
	Image::BitmapData bitmap_data(image, 0, 0, source.cols, source.rows, Image::BitmapData::ReadWriteMode::writeOnly);
	for (int row_index = 0; row_index < source.rows; row_index ++)
	{
		uint8_t * src_ptr = source.ptr(row_index);
		uint8_t * dst_ptr = bitmap_data.getLinePointer(row_index);

		std::memcpy(dst_ptr, src_ptr, number_of_bytes_to_copy);
	}

	return image;
}


dm::DMCanvas::DMCanvas() :
	invalid_point(-1, -1),
	invalid_rectangle(-1, -1, -1, -1),
	mouse_down_point(invalid_point),
	mouse_current_loc(invalid_point),
	mouse_previous_loc(invalid_point),
	mouse_drag_rectangle(invalid_rectangle),
	zoom(1.0)
{
	// why does this not work?
	setMouseCursor(MouseCursor::NoCursor);

	addMouseListener(this, false);

	return;
}


dm::DMCanvas::~DMCanvas()
{
	return;
}


void dm::DMCanvas::paint(Graphics & g)
{
	if (dmapp().darkhelp && current_image.empty())
	{
		current_image = cv::imread("../../DarkHelp/build/barcode_3.jpg");
		darkhelp().predict(current_image);
		darkhelp().annotate();
		current_img = convert_opencv_mat_to_juce_image(darkhelp().annotated_image);
	}

	if (current_img.isValid())
	{
		g.setOpacity(1.0f);
		const double w = zoom * double(getWidth());
		const double h = zoom * double(getHeight());
		g.drawImage(current_img, 0, 0, getWidth(), getHeight(), 0, 0, w, h);//current_img.getWidth(), current_img.getHeight());
	}
	else
	{
		g.fillAll(Colours::lightgreen);
	}

	g.setColour(Colours::blue);
	for (const auto & p : click_points)
	{
		g.drawEllipse(p.x, p.y, 4, 4, 1);
	}

	if (mouse_drag_rectangle != invalid_rectangle)
	{
		g.setColour(Colours::black);

		// The problem is if the user is moving the mouse upwards, or to the left, then we have a negative width
		// and/or height.  If that is the case, we need to reverse the two points because when given a negative
		// size, drawRect() doesn't draw anything.

		int x1 = mouse_drag_rectangle.getX();
		int x2 = mouse_drag_rectangle.getWidth() + x1;
		int y1 = mouse_drag_rectangle.getY();
		int y2 = mouse_drag_rectangle.getHeight() + y1;

		if (x1 > x2) { std::swap(x1, x2); }
		if (y1 > y2) { std::swap(y1, y2); }

		juce::Rectangle<int> r(x1, y1, x2 - x1, y2 - y1);
		g.drawRect(r, 1);
	}
	else if (mouse_current_loc != invalid_point)
	{
		g.setColour(Colours::red);
		g.drawHorizontalLine(mouse_current_loc.y, 0, getWidth());
		g.drawVerticalLine(mouse_current_loc.x, 0, getHeight());
	}

	return;
}


void dm::DMCanvas::mouseMove(const MouseEvent & event)
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


void dm::DMCanvas::mouseEnter(const MouseEvent & event)
{
	mouse_previous_loc	= invalid_point;
	mouse_current_loc	= event.getPosition();

	const int w = getWidth();
	const int h = getHeight();
	repaint(mouse_current_loc.x, 0, 1, h);
	repaint(0, mouse_current_loc.y, w, 1);

	return;
}


void dm::DMCanvas::mouseExit(const MouseEvent & event)
{
	mouse_previous_loc	= mouse_current_loc;
	mouse_current_loc	= invalid_point;

	const int w = getWidth();
	const int h = getHeight();
	repaint(mouse_previous_loc.x, 0, 1, h);
	repaint(0, mouse_previous_loc.y, w, 1);

	return;
}


void dm::DMCanvas::mouseDown(const MouseEvent & event)
{
	mouse_previous_loc		= mouse_current_loc;
	mouse_current_loc		= event.getPosition();

	mouse_drag_rectangle.setPosition(mouse_current_loc);
	mouse_drag_rectangle.setSize(0, 0);

	return;
}


void dm::DMCanvas::mouseUp(const MouseEvent & event)
{
	mouse_drag_rectangle	= invalid_rectangle;
	mouse_previous_loc		= mouse_current_loc;
	mouse_current_loc		= event.getPosition();

	cv::Point p(event.x, event.y);

	click_points.push_back(p);

	repaint();

	return;
}


void dm::DMCanvas::mouseDrag(const MouseEvent & event)
{
	mouse_previous_loc		= mouse_current_loc;
	mouse_current_loc		= event.getPosition();

	const auto p1 = mouse_drag_rectangle.getTopLeft();
	const auto p2 = mouse_current_loc;
	const auto w = p2.x - p1.x;
	const auto h = p2.y - p1.y;

	mouse_drag_rectangle.setWidth(w);
	mouse_drag_rectangle.setHeight(h);

	repaint();

	return;
}


void dm::DMCanvas::mouseWheelMove(const MouseEvent & event, const MouseWheelDetails & wheel)
{
	if (wheel.deltaY < 0)
	{
		zoom *= 1.1;
		repaint();
	}
	else if (wheel.deltaY > 0)
	{
		zoom /= 1.1;
		repaint();
	}

	// limit zoom to some decent values
	if (zoom > 6.7275000) { zoom = 6.7275000; }
	if (zoom < 0.0520987) { zoom = 0.0520987; }

	std::cout << "zoom=" << zoom << std::endl;

	return;
}
