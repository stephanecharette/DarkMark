/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMCanvas::DMCanvas(DMContent & c) :
	content(c)
{
	setName("ImageCanvas");

	mouse_drag_is_enabled = true;

	return;
}


dm::DMCanvas::~DMCanvas()
{
	return;
}


void dm::DMCanvas::rebuild_cache_image()
{
//	Log("redrawing layers...");

	//						blue   green red
	const cv::Scalar black	(0x00, 0x00, 0x00);
//	const cv::Scalar white	(0xff, 0xff, 0xff);

	if (content.original_image.empty())
	{
		// nothing we can do
		return;
	}

	content.scaled_image = resize_keeping_aspect_ratio(content.original_image, content.scaled_image_size);

	if (content.marks.empty() == false)
	{
		const auto fontface			= cv::FONT_HERSHEY_PLAIN;
		const auto fontscale		= 1.0;
		const auto fontthickness	= 1;

		for (size_t idx = 0; idx < content.marks.size(); idx ++)
		{
			const bool is_selected = (static_cast<int>(idx) == content.selected_mark);

			Mark & m = content.marks.at(idx);

			const std::string name	= m.description;
			const cv::Rect r		= m.get_bounding_rect(content.scaled_image.size());
			const cv::Scalar colour	= m.get_colour();
			const int thickness		= (is_selected ? 2 : 1);

			cv::Mat tmp = content.scaled_image(r).clone();
			cv::rectangle(tmp, cv::Rect(0, 0, tmp.cols, tmp.rows), colour, thickness, cv::LINE_8);

			double alpha = (is_selected ? 1.0 : 0.5);
			double beta = 1.0 - alpha;
			cv::addWeighted(tmp, alpha, content.scaled_image(r), beta, 0, content.scaled_image(r));

			if (is_selected)
			{
				tmp = content.scaled_image(r);
				cv::circle(tmp, cv::Point(0				, 0				), 10, colour, CV_FILLED, cv::LINE_AA);
				cv::circle(tmp, cv::Point(tmp.cols - 1	, 0				), 10, colour, CV_FILLED, cv::LINE_AA);
				cv::circle(tmp, cv::Point(tmp.cols - 1	, tmp.rows - 1	), 10, colour, CV_FILLED, cv::LINE_AA);
				cv::circle(tmp, cv::Point(0				, tmp.rows - 1	), 10, colour, CV_FILLED, cv::LINE_AA);
			}

			int baseline = 0;
			auto text_size = cv::getTextSize(name, fontface, fontscale, fontthickness, &baseline);

			// slide the label to the right so it lines up with the right-hand-side border of the bounding rect
			const int x_offset = r.width - text_size.width - 2;

			// Rectangle for the label needs the TL and BR coordinates.
			// But putText() needs the BL point where to start writing the text, and we want to add a 1x1 pixel border
			cv::Rect text_rect = cv::Rect(x_offset + r.x, r.y, text_size.width + 2, text_size.height + 4);
			if (is_selected)
			{
				text_rect.y = r.y - text_size.height - 3;
			}

			if (text_rect.x < 0) text_rect.x = 0;
			if (text_rect.y < 0) text_rect.y = 0;

			tmp = cv::Mat(text_rect.size(), CV_8UC3, colour);
			cv::putText(tmp, name, cv::Point(1, tmp.rows - 2), fontface, fontscale, black, fontthickness, cv::LINE_AA);

			cv::addWeighted(tmp, alpha, content.scaled_image(text_rect), beta, 0, content.scaled_image(text_rect));
		}
	}

	cached_image = convert_opencv_mat_to_juce_image(content.scaled_image);
	need_to_rebuild_cache_image = false;

	return;
}


void dm::DMCanvas::mouseUp(const MouseEvent & event)
{
	if (mouse_drag_is_enabled == false or mouse_drag_rectangle == invalid_rectangle)
	{
		const auto pos = event.getPosition();
		const cv::Point p(pos.x, pos.y);

		for (size_t idx = 0; idx < content.marks.size(); idx ++)
		{
			Mark & m = content.marks.at(idx);
			const cv::Rect r = m.get_bounding_rect(content.scaled_image_size);
			if (r.contains(p))
			{
				content.selected_mark = idx;
				content.rebuild_image_and_repaint();
				break;
			}
		}
	}

	CrosshairComponent::mouseUp(event);
	return;
}


void dm::DMCanvas::mouseDragFinished(juce::Rectangle<int> drag_rect)
{
	const double midx			= drag_rect.getCentreX();
	const double midy			= drag_rect.getCentreY();
	const double width			= drag_rect.getWidth();
	const double height			= drag_rect.getHeight();
	const double image_width	= cached_image.getWidth();
	const double image_height	= cached_image.getHeight();

#if 0
	Log("mouse drag rectangle:"
		" midx="			+ std::to_string(midx			) +
		" midy="			+ std::to_string(midy			) +
		" width="			+ std::to_string(width			) +
		" height="			+ std::to_string(height			) +
		" image_width="		+ std::to_string(image_width	) +
		" image_height="	+ std::to_string(image_height	));
#endif

	const int class_idx = 0;
	Mark m(	cv::Point2d(midx/image_width, midy/image_height),
			cv::Size2d(width/image_width, height/image_height),
			content.original_image.size(), class_idx);

	m.name			= content.names.at(class_idx);
	m.description	= m.name;

	content.marks.push_back(m);
	content.selected_mark = content.marks.size() - 1;
	content.rebuild_image_and_repaint();
	content.need_to_save = true;

	return;
}
