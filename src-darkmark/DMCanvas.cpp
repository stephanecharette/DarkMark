// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"

dm::DMCanvas::DMCanvas(DMContent &c)
	: CrosshairComponent(c), content(c), dragStart(0, 0), dragCurrent(0, 0), selectionRect(0, 0, 0, 0)
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

	if (content.images_are_loading or content.original_image.empty())
	{
		// nothing we can do
		return;
	}

#if 0
	Log("rebuild_cache_image:"
		" empty=" + std::string(content.original_image.empty() ? "true" : "false") +
		" size=" + std::to_string(content.original_image.cols) + "x" + std::to_string(content.original_image.rows) +
		" scaled=" + std::to_string(content.scaled_image_size.width) + "x" + std::to_string(content.scaled_image_size.height)
		);
#endif

	cv::Mat image_to_use;

	if (content.black_and_white_mode_enabled)
	{
		content.create_threshold_image();

		image_to_use = content.black_and_white_image;
	}
	else
	{
		image_to_use = content.original_image;
	}

	if (image_to_use.size() != content.scaled_image_size)
	{
		content.scaled_image = DarkHelp::resize_keeping_aspect_ratio(image_to_use, content.scaled_image_size);
	}
	else
	{
		content.scaled_image = image_to_use.clone();
	}

	if (content.heatmap_enabled and not content.heatmap_image.empty())
	{
		cv::Mat heatmap = DarkHelp::fast_resize_ignore_aspect_ratio(content.heatmap_image, content.scaled_image.size());

		if (content.heatmap_visualize < 0)
		{
			cv::cvtColor(heatmap, heatmap, cv::COLOR_GRAY2BGR);
		}
		else
		{
			cv::applyColorMap(heatmap, heatmap, content.heatmap_visualize);
		}

		const double alpha = 1.0 - content.heatmap_alpha_blend;
		const double beta = 1.0 - alpha;
		cv::addWeighted(content.scaled_image, alpha, heatmap, beta, 0, content.scaled_image);
	}

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

	bool mouse_drag_is_active = false;
	if (mouse_drag_rectangle != invalid_rectangle)
	{
		// user is dragging the mouse to create a point, so hide predictions
//		Log("dragging detected, turning off predictons in image cache");
		content.predictions_are_shown = false;
		mouse_drag_is_active = true;
	}

	if (content.number_of_marks == 0 and content.image_is_completely_empty == false)
	{
		content.marks_are_shown = false;
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
		const int thickness		= (mouse_drag_is_active == false and (is_selected or content.all_marks_are_bold) ? 2 : 1);

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

		const double alpha = (mouse_drag_is_active == false and (is_selected or content.all_marks_are_bold) ? 1.0 : content.alpha_blend_percentage);
		const double beta = 1.0 - alpha;
		cv::addWeighted(tmp, alpha, content.scaled_image(r), beta, 0, content.scaled_image(r));

		// draw the drag corners (only if the annotation is large enough to accomodate them)
		if (is_selected && tmp.cols > content.corner_size*2 && tmp.rows > content.corner_size*2)
		{
			tmp = content.scaled_image(r);
			cv::circle(tmp, cv::Point(0				, 0				), content.corner_size, colour, CV_FILLED, cv::LINE_AA);
			cv::circle(tmp, cv::Point(tmp.cols - 1	, 0				), content.corner_size, colour, CV_FILLED, cv::LINE_AA);
			cv::circle(tmp, cv::Point(tmp.cols - 1	, tmp.rows - 1	), content.corner_size, colour, CV_FILLED, cv::LINE_AA);
			cv::circle(tmp, cv::Point(0				, tmp.rows - 1	), content.corner_size, colour, CV_FILLED, cv::LINE_AA);
		}

		// We calculate the width and height of the text compared to the mark rectangle to determine if the label
		// would end up being bigger than the mark.  If the label is >= the size of the mark rectangle, then
		// we'll skip displaying the label when the mode is set to "auto".
		int baseline = 0;
		const auto text_size = cv::getTextSize(name, fontface, fontscale, fontthickness, &baseline);

		// draw the label (if the area is large enough to warrant a label)
		if	(mouse_drag_is_active == false and
				(content.show_labels == EToggle::kOn	or
				(content.show_labels == EToggle::kAuto	and
					(is_selected or
						(	text_size.width		<= tmp.cols and
							text_size.height	<= tmp.rows
						)
					)
				)))
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
			if (text_rect.x + text_rect.width >= content.scaled_image.cols)	text_rect.x = content.scaled_image.cols - text_rect.width;
			if (text_rect.x < 0) text_rect.x = 0;				// ...and if that didn't work, slide it to the left edge
			if (text_rect.x + text_rect.width >= content.scaled_image.cols) text_rect.width = content.scaled_image.cols - text_rect.x;

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

	if (content.user_specified_zoom_factor <= 0.0)
	{
		zoom_image_offset = cv::Point(0, 0);
	}
	else
	{
		// figure out what zoom offset we need to apply to the image

		const int h = content.canvas.getHeight();
		const int w = content.canvas.getWidth();

#if 0
		Log(std::string(__PRETTY_FUNCTION__) + ": need to find a RoI because we're zooming " + std::to_string(content.user_specified_zoom_factor) +
			", original image measures " +
			std::to_string(content.original_image.cols) +
			" x " +
			std::to_string(content.original_image.rows) +
			", scaled image measures " +
			std::to_string(content.scaled_image.cols) +
			" x " +
			std::to_string(content.scaled_image.rows) +
			" canvas measures"
			" w=" + std::to_string(w) +
			" h=" + std::to_string(h)
			);
#endif

		cv::Rect r(
			content.user_specified_zoom_factor * content.zoom_point_of_interest.x - w / 2,
			content.user_specified_zoom_factor * content.zoom_point_of_interest.y - h / 2,
			w, h);

		if (r.width < std::min(content.scaled_image.cols, w))
		{
			// our rectangle can be made wider to include more of the image
			const double delta = (std::min(content.scaled_image.cols, w) - r.width) / 2.0;
			r.x -= delta;
			r.width += std::round(delta * 2.0);
		}
		if (r.height < std::min(content.scaled_image.rows, h))
		{
			const double delta = (std::min(content.scaled_image.rows, h) - r.height) / 2.0;
			r.y -= delta;
			r.height += std::round(delta * 2.0);
		}
		if (r.x > 0 and r.x < 100)
		{
			// near the left border...we may as well stick to the border
			r.x = 0;
		}
		if (r.y > 0 and r.y < 100)
		{
			// near the top border...we may as well stick to the border
			r.y = 0;
		}
		if (r.x < 0)
		{
			r.width -= r.x;
			r.x = 0;
		}
		if (r.y < 0)
		{
			r.height -= r.y;
			r.y = 0;
		}

		// we now have a rectangle that would fit the canvas -- but is the image large enough to accomodate this rectangle?

		if (r.x + r.width > content.scaled_image.cols)
		{
			r.x = std::max(0, content.scaled_image.cols - r.width);
			r.width = content.scaled_image.cols - r.x;
		}
		if (r.y + r.height > content.scaled_image.rows)
		{
			r.y = std::max(0, content.scaled_image.rows - r.height);
			r.height = content.scaled_image.rows - r.y;
		}

#if 0
		Log(std::string(__PRETTY_FUNCTION__) + ": zoom=" + std::to_string(content.user_specified_zoom_factor) +
			" r.x=" + std::to_string(r.x) +
			" r.y=" + std::to_string(r.y) +
			" r.w=" + std::to_string(r.width) +
			" r.h=" + std::to_string(r.height) +
			" canvas.width=" + std::to_string(w) +
			" canvas.height=" + std::to_string(h));
#endif

		// this next line is what tells the crosshair component what portion of the image we want to show on the screen
		// zoom_image_offset = r.tl();
	}

//	Log(std::string(__PRETTY_FUNCTION__) + ": zoom image offset: x=" + std::to_string(zoom_image_offset.x) + " y=" + std::to_string(zoom_image_offset.y));

	int next_text_row = 25;
	if (content.predictions_are_shown and content.show_processing_time and content.darknet_image_processing_time.empty() == false)
	{
		cv::putText(content.scaled_image, content.darknet_image_processing_time, zoom_image_offset + cv::Point(10, next_text_row), fontface, fontscale, white, fontthickness, cv::LINE_AA);
		next_text_row += 15;
		cv::putText(content.scaled_image, "predictions: " + std::to_string(content.number_of_predictions), zoom_image_offset + cv::Point(10, next_text_row), fontface, fontscale, white, fontthickness, cv::LINE_AA);
		next_text_row += 15;
		if (number_of_hidden_marks)
		{
			cv::putText(content.scaled_image, "user marks: " + std::to_string(number_of_hidden_marks), zoom_image_offset + cv::Point(10, next_text_row), fontface, fontscale, white, fontthickness, cv::LINE_AA);
			next_text_row += 15;
		}
	}

	if (content.user_specified_zoom_factor > 0.0)
	{
		const int percentage = std::round(content.user_specified_zoom_factor * 100.0);
		cv::putText(content.scaled_image, "zoom: " + std::to_string(percentage) + "%", zoom_image_offset + cv::Point(10, next_text_row), fontface, fontscale, white, fontthickness, cv::LINE_AA);
		next_text_row += 15;
	}

	cached_image = convert_opencv_mat_to_juce_image(content.scaled_image);
	need_to_rebuild_cache_image = false;

	return;
}

void dm::DMCanvas::mouseDown(const MouseEvent &event)
{
	// Let the base crosshair see the event. That way if we do a fallback bounding box, it’s ready.
	CrosshairComponent::mouseDown(event);

	// 1) SHIFT => panning
	if (event.mods.isShiftDown())
	{
		panning_active = true;
		pan_start = event.getPosition();
		saved_offset = juce::Point<int>(
			content.canvas.zoom_image_offset.x,
			content.canvas.zoom_image_offset.y);

		setMouseCursor(MouseCursor::DraggingHandCursor);
		return; // skip everything else
	}

	// 2) Mass‐delete rubber‐band
	if (content.mass_delete_mode_active)
	{
		dragStart = event.getPosition();
		dragCurrent = dragStart;
		selectionRect = cv::Rect();
		return; // skip everything else
	}

	// 3) If user right‐clicked anywhere, we want to show a popup menu (like old code).
	//    We'll do it at the end so we can still pick a mark first if needed.

	// 4) Convert to “scaled‐image” coords
	const cv::Point p(
		mouse_current_loc.x + zoom_image_offset.x,
		mouse_current_loc.y + zoom_image_offset.y);

	// Save old selection
	const auto previous_selected_mark = content.selected_mark;
	content.selected_mark = -1;

	// For overlapping marks, choose the smallest area
	int smallest_area = INT_MAX;
	int best_idx = -1; // which mark we might select

	// 5) The user might be inside the “selected” mark => corner resizing or entire reposition
	//    We'll handle that first if they are indeed inside the selected mark.

	// Step (a): If there is a valid selected_mark, check if user clicked inside it:
	if (previous_selected_mark >= 0 &&
		previous_selected_mark < (int)content.marks.size())
	{
		Mark &selMark = content.marks[previous_selected_mark];
		cv::Rect selRect = selMark.get_bounding_rect(content.scaled_image_size);

		if (selRect.contains(p))
		{
			// => user clicked inside the bounding box of the “currently selected” mark
			// Check if near a corner => resizing
			if (!selMark.is_prediction)
			{
				// find nearest corner
				auto best_corner = ECorner::kTL;
				double best_len = std::numeric_limits<double>::infinity();
				for (auto corner_type : {ECorner::kTL, ECorner::kTR, ECorner::kBR, ECorner::kBL})
				{
					cv::Point corner_pt = selMark.get_corner(corner_type);
					double dist = std::hypot(
						double(corner_pt.x - p.x),
						double(corner_pt.y - p.y));
					if (dist < best_len)
					{
						best_len = dist;
						best_corner = corner_type;
					}
				}
				if (best_len <= content.corner_size)
				{
					// corner resizing
					resizingActive = true;
					resizingCorner = best_corner;
					resizingOriginalNormRect = selMark.get_normalized_bounding_rect();

					// selected_mark remains the same
					content.selected_mark = previous_selected_mark;
					content.most_recent_class_idx = selMark.class_idx;
					content.most_recent_size = selMark.get_normalized_bounding_rect().size();

					// If user right‐clicked corner, show menu
					if (event.mods.isPopupMenu())
					{
						content.create_popup_menu().showMenuAsync(PopupMenu::Options());
					}
					return; // skip fallback bounding box
				}
			}

			// If not near a corner => we want to “repositionActive” the entire bounding box
			repositionActive = true;
			repositionDragStart = event.getPosition();
			repositionOriginalNormRect = selMark.get_normalized_bounding_rect();

			// Keep it selected
			content.selected_mark = previous_selected_mark;
			content.most_recent_class_idx = selMark.class_idx;
			content.most_recent_size = selMark.get_normalized_bounding_rect().size();

			// If user right‐clicked inside the center, show menu
			if (event.mods.isPopupMenu())
			{
				content.create_popup_menu().showMenuAsync(PopupMenu::Options());
			}
			return; // skip bounding box creation
		}
	}

	// 6) If we get here, the user did NOT click inside the currently selected mark’s bounding box.
	//    So let's see if they clicked a different mark => we simply re‐select that mark.
	for (size_t idx = 0; idx < content.marks.size(); idx++)
	{
		// skip if it’s the old selected mark we already checked
		if ((int)idx == previous_selected_mark)
			continue;

		Mark &m = content.marks[idx];
		cv::Rect r = m.get_bounding_rect(content.scaled_image_size);

		if (r.contains(p))
		{
			// if user marks are shown or predictions are shown, we might pick it
			if ((content.marks_are_shown && !m.is_prediction) ||
				(content.predictions_are_shown && m.is_prediction))
			{
				int area = r.area();
				if (area < smallest_area)
				{
					smallest_area = area;
					best_idx = (int)idx;
				}
			}
		}
	}
	if (best_idx >= 0)
	{
		// We found a different mark => just select it. We do NOT reposition or resize it.
		content.selected_mark = best_idx;

		Mark &picked = content.marks[best_idx];
		content.most_recent_class_idx = picked.class_idx;
		content.most_recent_size = picked.get_normalized_bounding_rect().size();

		const auto &opencv_colour = content.annotation_colours.at(
			content.most_recent_class_idx % content.annotation_colours.size());
		content.crosshair_colour = Colour(opencv_colour[2], opencv_colour[1], opencv_colour[0]);
		content.rebuild_image_and_repaint();

		// If right‐click, show menu
		if (event.mods.isPopupMenu())
		{
			content.create_popup_menu().showMenuAsync(PopupMenu::Options());
		}
		return; // skip bounding box creation
	}

	// 7) If user right‐clicked empty space, show menu
	if (event.mods.isPopupMenu())
	{
		content.create_popup_menu().showMenuAsync(PopupMenu::Options());
		return; // done
	}

	// 8) If we STILL haven't done anything, we let the base crosshair code create a new bounding box if the user drags.
	//    We've already called CrosshairComponent::mouseDown(event) at the start,
	//    so the base class is ready to track the bounding box for mouseDrag().
	return;
}

void dm::DMCanvas::mouseDoubleClick(const MouseEvent & event)
{
#if 0
	if (content.most_recent_size.width < 0.02 or content.most_recent_size.height < 0.02)
	{
		content.most_recent_size = cv::Size2d(0.02, 0.02);
	}
#endif

	if (content.merge_mode_active)
	{
		content.selectMergeKeyFrame();
		return; // Skip normal mark creation.
	}

	double x = double(event.x + zoom_image_offset.x) / cached_image.getWidth();
	double y = double(event.y + zoom_image_offset.y) / cached_image.getHeight();

	Mark m(	cv::Point2d(x, y), content.most_recent_size, content.original_image.size(), content.most_recent_class_idx);
	m.name			= content.names.at(content.most_recent_class_idx);
	m.description	= m.name;

	content.marks.push_back(m);
	content.selected_mark = content.marks.size() - 1;
	content.need_to_save = true;
	content.image_is_completely_empty = false;
	if (content.snapping_enabled)
	{
		content.snap_annotation(content.selected_mark);
	}
	content.rebuild_image_and_repaint();

	return;
}


void dm::DMCanvas::mouseDragFinished(juce::Rectangle<int> drag_rect, const MouseEvent & event)
{

	// If we are currently repositioning an existing mark,
	// then we do NOT want to create a new bounding box at all.
	if (repositionActive)
	{
		// Just bail out early to avoid overwriting the user’s reposition
		// with the “new mark” logic that used to be here.
		return;
	}

	if (content.mass_delete_mode_active)
	{



		cv::Rect r(drag_rect.getX(), drag_rect.getY(), drag_rect.getWidth(), drag_rect.getHeight());
		content.handleMassDeleteArea(r);

		return; // skip the normal single-mark bounding-box code
	}

	double midx			= drag_rect.getCentreX() + zoom_image_offset.x;
	double midy			= drag_rect.getCentreY() + zoom_image_offset.y;
	double width		= drag_rect.getWidth();
	double height		= drag_rect.getHeight();
	double image_width	= cached_image.getWidth();
	double image_height	= cached_image.getHeight();

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

	bool snap = content.snapping_enabled;
	if (event.mods.isShiftDown())
	{
		// when SHIFT is held down we invert the "snap" setting
		snap = ! snap;
	}
	if (snap)
	{
		content.snap_annotation(content.selected_mark);
	}

	content.rebuild_image_and_repaint();

	return;
}

// DMCanvas.cpp

bool dm::DMCanvas::isInCenterRegion(const juce::Rectangle<int> &rect, int x, int y, double marginFactor) const
{
	// Define margins as a fraction of the rectangle’s dimensions.
	int marginX = static_cast<int>(rect.getWidth() * marginFactor);
	int marginY = static_cast<int>(rect.getHeight() * marginFactor);

	// Create a "central" region by leaving out margins from each side.
	juce::Rectangle<int> centerRegion(rect.getX() + marginX,
									  rect.getY() + marginY,
									  rect.getWidth() - 2 * marginX,
									  rect.getHeight() - 2 * marginY);
	return centerRegion.contains(x, y);
}

void dm::DMCanvas::mouseDrag(const MouseEvent &event)
{
	if (resizingActive)
	{
		// The user is dragging a corner
		Mark &selMark = content.marks[content.selected_mark];

		// Original bounding rect
		cv::Rect2d oldNormRect = resizingOriginalNormRect;

		// Convert the old rect from normalized → absolute coords (scaled image):
		double imgW = (double)content.scaled_image_size.width;
		double imgH = (double)content.scaled_image_size.height;

		cv::Rect2d oldAbsRect(
			oldNormRect.x * imgW,
			oldNormRect.y * imgH,
			oldNormRect.width * imgW,
			oldNormRect.height * imgH);

		// Current mouse in scaled coords
		cv::Point scaledPt(
			event.x + content.canvas.zoom_image_offset.x,
			event.y + content.canvas.zoom_image_offset.y);

		// We'll pin the opposite corner and move the corner being dragged:
		cv::Point2d newTL = oldAbsRect.tl();
		cv::Point2d newBR = oldAbsRect.br();

		switch (resizingCorner)
		{
		case ECorner::kTL:
			newTL.x = (double)scaledPt.x;
			newTL.y = (double)scaledPt.y;
			break;
		case ECorner::kTR:
			newTL.y = (double)scaledPt.y; // top
			newBR.x = (double)scaledPt.x; // right
			break;
		case ECorner::kBR:
			newBR.x = (double)scaledPt.x;
			newBR.y = (double)scaledPt.y;
			break;
		case ECorner::kBL:
			newTL.x = (double)scaledPt.x; // left
			newBR.y = (double)scaledPt.y; // bottom
			break;
		}

		// Ensure we have a valid rectangle with positive width/height
		double x1 = std::min(newTL.x, newBR.x);
		double y1 = std::min(newTL.y, newBR.y);
		double x2 = std::max(newTL.x, newBR.x);
		double y2 = std::max(newTL.y, newBR.y);

		cv::Rect2d newAbsRect(x1, y1, x2 - x1, y2 - y1);

		// Convert back to normalized
		cv::Rect2d newNorm(
			newAbsRect.x / imgW,
			newAbsRect.y / imgH,
			newAbsRect.width / imgW,
			newAbsRect.height / imgH);

		// If needed, clamp to [0..1]
		// e.g. newNorm.x = std::max(0.0, std::min(newNorm.x, 1.0 - newNorm.width));

		// Update the mark
		selMark.normalized_all_points = {
			{newNorm.x, newNorm.y},
			{newNorm.x + newNorm.width, newNorm.y},
			{newNorm.x + newNorm.width, newNorm.y + newNorm.height},
			{newNorm.x, newNorm.y + newNorm.height}};
		selMark.rebalance();

		// Repaint
		content.rebuild_image_and_repaint();
		return;
	}
	else if (repositionActive)
	{
		// Existing reposition logic
		juce::Point<int> delta = event.getPosition() - repositionDragStart;
		double ndx = (double)delta.x / (double)getWidth();
		double ndy = (double)delta.y / (double)getHeight();

		cv::Rect2d newNormRect = repositionOriginalNormRect;
		newNormRect.x += ndx;
		newNormRect.y += ndy;

		Mark &selMark = content.marks[content.selected_mark];
		selMark.normalized_all_points = {
			{newNormRect.x, newNormRect.y},
			{newNormRect.x + newNormRect.width, newNormRect.y},
			{newNormRect.x + newNormRect.width, newNormRect.y + newNormRect.height},
			{newNormRect.x, newNormRect.y + newNormRect.height}};
		selMark.rebalance();
		content.rebuild_image_and_repaint();
		return;
	}

	// Otherwise, fallback => new bounding box
	CrosshairComponent::mouseDrag(event);
}

void dm::DMCanvas::mouseUp(const MouseEvent &event)
{
	if (resizingActive)
	{
		resizingActive = false;
		// The user finished resizing, so let's reset any "mouse_drag_rectangle" if you used it
		mouse_drag_rectangle = invalid_rectangle;

		content.need_to_save = true;
		return;
	}
	else if (repositionActive)
	{
		repositionActive = false;
		// Possibly also reset "mouse_drag_rectangle" if you want
		mouse_drag_rectangle = invalid_rectangle;

		content.need_to_save = true;
		return;
	}

	CrosshairComponent::mouseUp(event);
}
