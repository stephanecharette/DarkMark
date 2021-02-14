// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMCanvas::DMCanvas(DMContent & c) :
	CrosshairComponent(c),
	content(c)
{
	setName("ImageCanvas");

	mouse_drag_is_enabled = true;

	setWantsKeyboardFocus(true);
	grabKeyboardFocus();

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
	const cv::Scalar white	(0xff, 0xff, 0xff);

	if (content.original_image.empty())
	{
		// nothing we can do
		return;
	}

	content.scaled_image = resize_keeping_aspect_ratio(content.original_image, content.scaled_image_size);

	const auto fontface			= cv::FONT_HERSHEY_PLAIN;
	const auto fontscale		= 1.0;
	const auto fontthickness	= 1;

	content.marks_are_shown			= content.show_marks;
	content.predictions_are_shown	= content.show_predictions != EToggle::kOff;
	content.number_of_marks			= 0;
	content.number_of_predictions	= 0;

	// Do we have any actual marks or should we show predictions?
	// If we show predictions, we should also let the user know how many "real" marks are hidden.
	size_t number_of_hidden_marks = 0;
	for (const auto & m : content.marks)
	{
		if (m.is_prediction == true)
		{
			content.number_of_predictions ++;
		}
		else
		{
			content.number_of_marks ++;

			if (content.marks_are_shown == false)
			{
				// this is a real mark which will be hidden from view
				number_of_hidden_marks ++;
			}
			else if (content.show_predictions == EToggle::kAuto)
			{
				// this is a real mark which will be shown, so turn off predictions
				content.predictions_are_shown = false;
			}
		}
	}

	if (mouse_drag_rectangle != invalid_rectangle)
	{
		// user is dragging the mouse to create a point, so hide predictions
		Log("dragging detected, turning off predictons in image cache");
		content.predictions_are_shown = false;
	}

	if (content.number_of_marks == 0 and content.image_is_completely_empty == false)
	{
		content.marks_are_shown = false;
	}

	if (content.predictions_are_shown and content.show_processing_time and content.darknet_image_processing_time.empty() == false)
	{
		cv::putText(content.scaled_image, content.darknet_image_processing_time, cv::Point(10, 25), fontface, fontscale, white, fontthickness, cv::LINE_AA);
		cv::putText(content.scaled_image, "predictions: " + std::to_string(content.number_of_predictions), cv::Point(10, 40), fontface, fontscale, white, fontthickness, cv::LINE_AA);
		if (number_of_hidden_marks)
		{
			cv::putText(content.scaled_image, "user marks: " + std::to_string(number_of_hidden_marks), cv::Point(10, 55), fontface, fontscale, white, fontthickness, cv::LINE_AA);
		}
	}

	bool must_exit_loop = false;

	for (size_t idx = 0; must_exit_loop == false and (content.image_is_completely_empty or idx < content.marks.size()); idx ++)
	{
		Mark m;
		if (content.image_is_completely_empty)
		{
			// treat empty images as if they have a single mark covering the entire image
			m = Mark(cv::Point2d(0.5, 0.5), cv::Size2d(1.0, 1.0), cv::Size(0, 0), content.empty_image_name_index);
			m.description = content.names[content.empty_image_name_index];

			// we're using a "fake" mark on empty images, so immediately exit this loop once we're done drawing what we need
			must_exit_loop = true;
		}
		else
		{
			m = content.marks.at(idx);
		}

		if (m.is_prediction == false and content.marks_are_shown == false)
		{
			continue;
		}

		if (m.is_prediction == true and content.predictions_are_shown == false)
		{
			continue;
		}

		const bool is_selected	= (static_cast<int>(idx) == content.selected_mark);
		const std::string name	= m.description;
		const cv::Rect r		= m.get_bounding_rect(content.scaled_image.size());
		const cv::Scalar colour	= m.get_colour();
		const int thickness		= (is_selected or content.all_marks_are_bold ? 2 : 1);

		cv::Mat tmp = content.scaled_image(r).clone();

		if (content.shade_rectangles)
		{
			const double shade_divider = (is_selected ? 4.0 : 2.0);
			cv::rectangle(tmp, cv::Rect(0, 0, tmp.cols, tmp.rows), colour, CV_FILLED);
			const double alpha = content.alpha_blend_percentage / shade_divider;
			const double beta = 1.0 - alpha;
			cv::addWeighted(tmp, alpha, content.scaled_image(r), beta, 0, tmp);
		}

		cv::rectangle(tmp, cv::Rect(0, 0, tmp.cols, tmp.rows), colour, thickness, cv::LINE_8);

		if (m.is_prediction)
		{
			// draw an "X" through the middle of the rectangle
			cv::line(tmp, cv::Point(0, 0), cv::Point(tmp.cols, tmp.rows), colour, 1, cv::LINE_8);
			cv::line(tmp, cv::Point(0, tmp.rows), cv::Point(tmp.cols, 0), colour, 1, cv::LINE_8);
		}

		const double alpha = (is_selected or content.all_marks_are_bold ? 1.0 : content.alpha_blend_percentage);
		const double beta = 1.0 - alpha;
		cv::addWeighted(tmp, alpha, content.scaled_image(r), beta, 0, content.scaled_image(r));

		// draw the drag corners
		if (is_selected and tmp.cols >= 30 and tmp.rows >= 30)
		{
			tmp = content.scaled_image(r);
			cv::circle(tmp, cv::Point(0				, 0				), 10, colour, CV_FILLED, cv::LINE_AA);
			cv::circle(tmp, cv::Point(tmp.cols - 1	, 0				), 10, colour, CV_FILLED, cv::LINE_AA);
			cv::circle(tmp, cv::Point(tmp.cols - 1	, tmp.rows - 1	), 10, colour, CV_FILLED, cv::LINE_AA);
			cv::circle(tmp, cv::Point(0				, tmp.rows - 1	), 10, colour, CV_FILLED, cv::LINE_AA);
		}

		// We calculate the width and height of the text compared to the mark rectangle to determine if the label
		// would end up being bigger than the mark.  If the label is >= the size of the mark rectangle, then
		// we'll skip displaying the label when the mode is set to "auto".
		int baseline = 0;
		const auto text_size = cv::getTextSize(name, fontface, fontscale, fontthickness, &baseline);

		// draw the label (if the area is large enough to warrant a label)
		if	(content.show_labels == EToggle::kOn	or
			(content.show_labels == EToggle::kAuto	and
				(is_selected or
					(	text_size.width		<= tmp.cols and
						text_size.height	<= tmp.rows
					)
				)
			))
		{
			// slide the label to the right so it lines up with the right-hand-side border of the bounding rect
			const int x_offset = r.width - text_size.width - 2;

			// Rectangle for the label needs the TL and BR coordinates.
			// But putText() needs the BL point where to start writing the text, and we want to add a 1x1 pixel border
			cv::Rect text_rect = cv::Rect(x_offset + r.x, r.y, text_size.width + 2, text_size.height + baseline + 2);
			if (is_selected or content.all_marks_are_bold or text_rect.br().y >= content.scaled_image.rows)
			{
				// move the text label above the rectangle
				text_rect.y = r.y - text_size.height - baseline;
			}

#if 0
			Log("scaled image cols=" + std::to_string(content.scaled_image.cols) + " rows=" + std::to_string(content.scaled_image.rows));
			Log("text size for " + name + ": w=" + std::to_string(text_size.width) + " h=" + std::to_string(text_size.height) + " baseline=" + std::to_string(baseline));
			Log("mark rectangle:  "
				" x=" + std::to_string(r.x) +
				" y=" + std::to_string(r.y) +
				" w=" + std::to_string(r.width) +
				" h=" + std::to_string(r.height));
			Log("text_rect before:"
				" x=" + std::to_string(text_rect.x) +
				" y=" + std::to_string(text_rect.y) +
				" w=" + std::to_string(text_rect.width) +
				" h=" + std::to_string(text_rect.height));
#endif
			// check to see if the label is going to be off-screen, and if so slide it to a better position
			if (text_rect.x < 0) text_rect.x = r.x;				// first attempt to fix this is to make it left-aligned
			if (text_rect.x < 0) text_rect.x = 0;				// ...and if that didn't work, slide it to the left edge
			if (text_rect.y < 0) text_rect.y = r.y + r.height;	// vertically, we need to place the label underneath instead of above

			// if the mark is from the top of the image to the bottom of the image, then we still haven't
			// found a good place to put the label, in which case we'll move it to a spot inside the mark
			if (text_rect.y + r.height >= content.scaled_image.rows) text_rect.y = r.y + 2;

#if 0
			Log("text_rect after: "
				" x=" + std::to_string(text_rect.x) +
				" y=" + std::to_string(text_rect.y) +
				" w=" + std::to_string(text_rect.width) +
				" h=" + std::to_string(text_rect.height));
#endif

			tmp = cv::Mat(text_rect.size(), CV_8UC3, colour);
			cv::putText(tmp, name, cv::Point(1, tmp.rows - 5), fontface, fontscale, black, fontthickness, cv::LINE_AA);
			cv::addWeighted(tmp, alpha, content.scaled_image(text_rect), beta, 0, content.scaled_image(text_rect));
		}
	}

	cached_image = convert_opencv_mat_to_juce_image(content.scaled_image);
	need_to_rebuild_cache_image = false;

	return;
}


void dm::DMCanvas::mouseDown(const MouseEvent & event)
{
	CrosshairComponent::mouseDown(event);

	const auto previous_selected_mark = content.selected_mark;

//	const auto shift_mod	= event.mods.isShiftDown();
	const auto pos			= event.getPosition();
	content.selected_mark	= -1;
	const cv::Point p(pos.x, pos.y);

	int index_to_delete = -1;

	// find all of the marks beneath the mouse location, and remember the one with the *smallest* area so
	// that if we have a tiny mark within a larger mark, this will select the smaller (harder to click) mark
	int smallest_area = INT_MAX;
	for (size_t idx = 0; index_to_delete == -1 and idx < content.marks.size(); idx ++)
	{
		Mark & m = content.marks.at(idx);
		const cv::Rect r = m.get_bounding_rect(content.scaled_image_size);
		if (r.contains(p))
		{
			// corner dragging should only be done on "real" marks, not predictions
			if (m.is_prediction == false)
			{
				// check to see if this mouse click is within 10 pixels from the corner
				for (const auto type : {ECorner::kTL, ECorner::kTR, ECorner::kBR, ECorner::kBL})
				{
					cv::Point corner_point = m.get_corner(type);
					const double len = std::round(std::hypot(corner_point.x - p.x, corner_point.y - p.y));
					if (len < 10)
					{
						// we've clicked on a corner!

						const auto opposite_corner = static_cast<ECorner>((static_cast<int>(type) + 2) % 4);
						const cv::Point corner_point = m.get_corner(opposite_corner);
						mouse_drag_rectangle.setPosition(corner_point.x, corner_point.y);
						index_to_delete = static_cast<int>(idx);
						break;
					}
				}
			}

			// if this isn't a corner click, see if we've clicked inside a mark
			if (index_to_delete == -1)
			{
				if ((content.marks_are_shown and m.is_prediction == false) or
					(content.predictions_are_shown and m.is_prediction))
				{
					const int area = r.area();
					if (area < smallest_area)
					{
						smallest_area = area;
						content.selected_mark = idx;
						content.most_recent_class_idx = m.class_idx;
						content.most_recent_size = m.get_normalized_bounding_rect().size();
					}
				}
			}
		}
	}

	if (index_to_delete >= 0)
	{
		content.most_recent_class_idx = content.marks[index_to_delete].class_idx;
		content.marks.erase(content.marks.begin() + index_to_delete);
		content.selected_mark = -1;
		content.need_to_save = true;
		mouseDrag(event);
	}

	if (previous_selected_mark != content.selected_mark)
	{
		const auto & opencv_colour = content.annotation_colours.at(content.most_recent_class_idx);
		content.crosshair_colour = Colour(opencv_colour[2], opencv_colour[1], opencv_colour[0]);
		content.rebuild_image_and_repaint();
	}

	if (event.mods.isPopupMenu())
	{
		content.create_popup_menu().showMenuAsync(PopupMenu::Options());
	}

	return;
}


void dm::DMCanvas::mouseDoubleClick(const MouseEvent & event)
{
	if (content.most_recent_size.width < 0.02 or content.most_recent_size.height < 0.02)
	{
		content.most_recent_size = cv::Size2d(0.02, 0.02);
	}

	const double image_width	= cached_image.getWidth();
	const double image_height	= cached_image.getHeight();
	const double x = double(event.x) / image_width;
	const double y = double(event.y) / image_height;
	Mark m(	cv::Point2d(x, y), content.most_recent_size, content.original_image.size(), content.most_recent_class_idx);

	m.name			= content.names.at(content.most_recent_class_idx);
	m.description	= m.name;

	content.marks.push_back(m);
	content.selected_mark = content.marks.size() - 1;
	content.need_to_save = true;
	content.image_is_completely_empty = false;
	content.rebuild_image_and_repaint();

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

	const int class_idx = content.most_recent_class_idx;
	Mark m(	cv::Point2d(midx/image_width, midy/image_height),
			cv::Size2d(width/image_width, height/image_height),
			content.original_image.size(), class_idx);

	m.name			= content.names.at(class_idx);
	m.description	= m.name;

	content.marks.push_back(m);
	content.selected_mark = content.marks.size() - 1;
	content.most_recent_size = m.get_normalized_bounding_rect().size();
	content.need_to_save = true;
	content.image_is_completely_empty = false;
	content.rebuild_image_and_repaint();

	return;
}
