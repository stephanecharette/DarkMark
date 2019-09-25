/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMCanvas::DMCanvas()
{
	mouse_drag_is_enabled = true;

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
		" - "	+ std::string("barcode_3.jpg") +
		" - "	+ std::to_string(static_cast<int>(image_width				)) +
		"x"		+ std::to_string(static_cast<int>(image_height				)) +
		" - "	+ std::to_string(static_cast<int>(std::round(factor * 100.0))) + "%";

	dmapp().wnd->setName(title);

	// now that the canvas has changed size we're going to have to rescale and rebuild the image
	CrosshairComponent::resized();

	return;
}


void dm::DMCanvas::rebuild_cache_image()
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

	cached_image = convert_opencv_mat_to_juce_image(composited_image);
	need_to_rebuild_cache_image = false;

	return;
}
