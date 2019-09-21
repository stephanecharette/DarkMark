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
	need_to_redraw_layers(false),
	invalid_point(-1, -1),
	invalid_rectangle(-1, -1, -1, -1),
	mouse_down_point(invalid_point),
	mouse_current_loc(invalid_point),
	mouse_previous_loc(invalid_point),
	mouse_drag_rectangle(invalid_rectangle),
	zoom_factor(1.0)
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


void dm::DMCanvas::resized()
{
	// given the new size, figure out what kind of scaling we need to do for the
	// entire image to be shown and update the titlebar to display the zoom factor

	const double image_width	= original_image.cols;
	const double image_height	= original_image.rows;
	const double canvas_width	= getWidth();
	const double canvas_height	= getHeight();

	if (image_width		< 1.0 ||
		image_height	< 1.0 ||
		canvas_width	< 1.0 ||
		canvas_height	< 1.0)
	{
		// window hasn't been created yet, or the image has not been loaded yet
		return;
	}

	const double factor = std::min(canvas_width / image_width, canvas_height / image_height);

	// since we're going to be messing around with the window title, make a copy of the original window name
	static const std::string original_title = dmapp().wnd->getName().toStdString();

	std::string title =
		original_title +
		" - "	+ std::string("barcode_3.jpg ") +
		" - "	+ std::to_string(static_cast<int>(image_width				)) +
		"x"		+ std::to_string(static_cast<int>(image_height				)) +
		" - "	+ std::to_string(static_cast<int>(std::round(factor * 100.0))) + "%";

	dmapp().wnd->setName(title);

	// now that the canvas has changed size we're going to have to rescale the image
	need_to_redraw_layers = true;
	repaint();

	return;
}


void dm::DMCanvas::paint(Graphics & g)
{
	if (need_to_redraw_layers)
	{
		redraw_layers();
	}

	if (final_img.isValid())
	{
		g.setOpacity(1.0f);
		const double w = zoom_factor * double(getWidth());
		const double h = zoom_factor * double(getHeight());
		g.drawImage(final_img, 0, 0, getWidth(), getHeight(), 0, 0, w, h);
	}
	else
	{
		g.fillAll(Colours::lightgreen);
	}

	if (mouse_drag_rectangle != invalid_rectangle)
	{
		g.setColour(Colours::red);

		// The problem is if the user is moving the mouse upwards, or to the left, then we have a negative width
		// and/or height.  If that is the case, we need to reverse the two points because when given a negative
		// size, drawRect() doesn't draw anything.

		int x1 = mouse_drag_rectangle.getX();
		int x2 = mouse_drag_rectangle.getWidth() + x1;
		int y1 = mouse_drag_rectangle.getY();
		int y2 = mouse_drag_rectangle.getHeight() + y1;

		// make sure x1, x2, y1, and y2 represent the TL and BR corners so the width and height will be a positive number
		if (x1 > x2) { std::swap(x1, x2); }
		if (y1 > y2) { std::swap(y1, y2); }

		juce::Rectangle<int> r(x1, y1, x2 - x1, y2 - y1);
		g.drawRect(r, 1);
	}
	else if (mouse_current_loc != invalid_point)
	{
		g.setColour(Colours::red);
		g.drawHorizontalLine(	mouse_current_loc.y, 0, getWidth()	);
		g.drawVerticalLine(		mouse_current_loc.x, 0, getHeight()	);
	}

	return;
}


void dm::DMCanvas::mouseMove(const MouseEvent & event)
{
	// record where the mouse is and invalidate the drawing
	mouse_previous_loc	= mouse_current_loc;
	mouse_current_loc	= event.getPosition();

#if 0
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
#else
	repaint();
#endif

	return;
}


void dm::DMCanvas::mouseEnter(const MouseEvent & event)
{
	need_to_redraw_layers = true;

	mouse_previous_loc	= invalid_point;
	mouse_current_loc	= event.getPosition();

#if 0
	const int w = getWidth();
	const int h = getHeight();
	repaint(mouse_current_loc.x, 0, 1, h);
	repaint(0, mouse_current_loc.y, w, 1);
#else
	repaint();
#endif

	return;
}


void dm::DMCanvas::mouseExit(const MouseEvent & event)
{
	mouse_previous_loc	= mouse_current_loc;
	mouse_current_loc	= invalid_point;

#if 0
	const int w = getWidth();
	const int h = getHeight();
	repaint(mouse_previous_loc.x, 0, 1, h);
	repaint(0, mouse_previous_loc.y, w, 1);
#else
	repaint();
#endif

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

	if (zoom_factor != 1.0) mouse_current_loc.x *= zoom_factor;
	if (zoom_factor != 1.0) mouse_current_loc.y *= zoom_factor;

	cv::Point p(mouse_current_loc.x, event.y);

	click_points.push_back(p);

	need_to_redraw_layers = true;
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
		zoom_factor *= 1.1;
		need_to_redraw_layers = true;
		repaint();
	}
	else if (wheel.deltaY > 0)
	{
		zoom_factor /= 1.1;
		need_to_redraw_layers = true;
		repaint();
	}

	// limit zoom to some decent values
	if (zoom_factor > 6.7275000) { zoom_factor = 6.7275000; }
	if (zoom_factor < 0.0520987) { zoom_factor = 0.0520987; }

	std::cout << "zoom=" << zoom_factor << std::endl;

	return;
}


dm::DMCanvas & dm::DMCanvas::redraw_layers()
{
	Log("redrawing layers...");

	//						blue   green red
	const cv::Scalar black	(0x00, 0x00, 0x00);
	const cv::Scalar blue	(0xff, 0x00, 0x00);
	const cv::Scalar purple	(0xff, 0x00, 0xff);
	const cv::Scalar white	(0xff, 0xff, 0xff);

	if (dmapp().darkhelp and original_image.empty())
	{
		const std::string filename = "/home/stephane/src/DarkHelp/build/barcode_3.jpg";
		Log("calling darkhelp to process the image " + filename);
		try
		{
			original_image = cv::imread(filename);
			darkhelp().predict(original_image);
			darkhelp().annotate();
			Log("darkhelp processed the image in " + darkhelp().duration_string());
		}
		catch(...)
		{
			Log("Error: failed to process image " + filename);
		}
	}

	scaled_image = resize_keeping_aspect_ratio(original_image, cv::Size(getWidth(), getHeight()));

	layer_background_image = scaled_image;

	cv::Mat layer_darkhelp_mask;
	cv::Mat layer_class_names_mask;
	layer_darkhelp = cv::Mat();
	layer_class_names = cv::Mat();

	if (darkhelp().prediction_results.empty() == false)
	{
		layer_darkhelp			= cv::Mat::zeros(layer_background_image.size(), CV_8UC3);
		layer_darkhelp_mask		= cv::Mat::zeros(layer_background_image.size(), CV_8UC1);
		layer_class_names		= cv::Mat::zeros(layer_background_image.size(), CV_8UC3);
		layer_class_names_mask	= cv::Mat::zeros(layer_background_image.size(), CV_8UC1);

		const auto fontface			= cv::FONT_HERSHEY_PLAIN;
		const auto fontscale		= 1.0;
		const auto fontthickness	= 1;

		for (const auto pred : darkhelp().prediction_results)
		{
			// the image might have been scaled, so rebuild the prediction rectangle with the new coordinates
			const int w = std::round(pred.width		* layer_background_image.cols);
			const int h = std::round(pred.height	* layer_background_image.rows);
			const int x = std::round(pred.mid_x		* layer_background_image.cols - w / 2.0f);
			const int y = std::round(pred.mid_y		* layer_background_image.rows - h / 2.0f);
			cv::Rect r(x, y, w, h);
			cv::rectangle(layer_darkhelp		, r, purple	, 1, cv::LINE_8);
			cv::rectangle(layer_darkhelp_mask	, r, white	, 1, cv::LINE_8);

			int baseline = 0;
			auto text_size = cv::getTextSize(pred.name, fontface, fontscale, fontthickness, &baseline);

			// slide the barcode to the right so it lines up with the right-hand-side border of the bounding rect
			const int x_offset = r.width - text_size.width - 2;

			// Rectangle for the label needs the TL and BR coordinates.
			// But putText() needs the BL point where to start writing the text, and we want to add a 1x1 pixel border
			const cv::Point text_point = r.tl() + cv::Point(x_offset + 1, -2);
			const cv::Rect text_rect(x_offset + r.x, r.y - text_size.height - 3, text_size.width + 2, text_size.height + 3);
			cv::rectangle(layer_class_names, text_rect, purple, cv::FILLED, cv::LINE_8);
			cv::rectangle(layer_class_names_mask, text_rect, white, cv::FILLED, cv::LINE_8);
			cv::putText(layer_class_names, pred.name, text_point, fontface, fontscale, white, fontthickness, cv::LINE_AA);
		}
	}

	cv::Mat layer_points_of_interest_mask;
	layer_points_of_interest = cv::Mat();
	if (not click_points.empty())
	{
		layer_points_of_interest		= cv::Mat::zeros(layer_background_image.size(), CV_8UC3);
		layer_points_of_interest_mask	= cv::Mat::zeros(layer_background_image.size(), CV_8UC1);

		for (const auto & p : click_points)
		{
			Log("adding point " + std::to_string(p.x) + "," + std::to_string(p.y) + " to the points-of-interest layer");
			cv::circle(layer_points_of_interest		, cv::Point(p.x, p.y), 2, blue	, cv::FILLED, cv::LINE_8);
			cv::circle(layer_points_of_interest_mask, cv::Point(p.x, p.y), 2, white	, cv::FILLED, cv::LINE_8);
		}
	}

	// time to add all the layers together
	composited_image = layer_background_image.clone();
	if (layer_darkhelp.empty() == false and layer_darkhelp_mask.empty() == false)
	{
		Log("adding darkhelp layer to composited image");
		layer_darkhelp.copyTo(composited_image, layer_darkhelp_mask);
	}
	if (layer_class_names.empty() == false and layer_class_names_mask.empty() == false)
	{
		Log("adding class names layer to composited image");

		cv::Mat tmp = composited_image.clone();
		layer_class_names.copyTo(tmp, layer_class_names_mask);

		const double alpha = 1.0;
		const double beta = 1.0 - alpha;
		cv::addWeighted(tmp, alpha, composited_image, beta, 0, composited_image);
	}
	if (layer_points_of_interest.empty() == false and layer_points_of_interest_mask.empty() == false)
	{
		Log("adding points-of-interest layer to composited image");
		layer_points_of_interest.copyTo(composited_image, layer_points_of_interest_mask);
	}

	final_img = convert_opencv_mat_to_juce_image(composited_image);

	Log("clearing the need_to_redraw_layers flag");
	need_to_redraw_layers = false;

	return *this;
}
