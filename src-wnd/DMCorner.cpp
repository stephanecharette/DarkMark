/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMCorner::DMCorner(DMContent & c, const ECorner type) :
	content(c),
	corner(type),
	cell_size(20),
	cols(10),
	rows(10)
//	offset(0, 0),
//	poi(0, 0)
{
	mouse_drag_is_enabled = false;

	return;
}


dm::DMCorner::~DMCorner()
{
	return;
}


void dm::DMCorner::mouseDown(const MouseEvent & event)
{
	CrosshairComponent::mouseDown(event);

	if (event.mods.isPopupMenu())
	{
		PopupMenu classes;
		classes.addItem("barcode"		, std::function<void()>( [=]{ this->set_class(0); } ));
		classes.addItem("aisle-bay-loc"	, std::function<void()>( [=]{ this->set_class(1); } ));

		PopupMenu m;
		m.addSubMenu("classes", classes);

		m.addItem("testing"	, std::function<void()>( [=]{ this->set_class(0); } ));
		m.addItem("blah"	, std::function<void()>( [=]{ this->set_class(0); } ));
		m.showMenuAsync(PopupMenu::Options());
	}
	else
	{
		const int x = event.getMouseDownX();
		const int y = event.getMouseDownY();
		const int cell_x = x / (cell_size + 1);
		const int cell_y = y / (cell_size + 1);
		Log("mouse down at x=" + std::to_string(x) + " y=" + std::to_string(y) + " cell=" + std::to_string(cell_x) + "," + std::to_string(cell_y));

		cv::Point p = top_left_point;
		p.x += cell_x;
		p.y += cell_y;

		Log("corner loc is: " + std::to_string(top_left_point.x) + "," + std::to_string(top_left_point.y));
		Log("new poi loc is " + std::to_string(p.x) + "," + std::to_string(p.y));

		content.marks[content.selected_mark].set(corner, p);
		content.rebuild_image_and_repaint();
	}

	return;
}


void dm::DMCorner::rebuild_cache_image()
{
	const int h = getHeight();
	const int w = getWidth();

	cv::Mat mat;
	if (content.original_image.empty() or content.selected_mark < 0)
	{
		// nothing we can do if we don't have an image to show, so draw a blank bright green background
		mat = cv::Mat(h, w, CV_8UC3, cv::Scalar(0x80, 0xff, 0x80));

		// draw vertical grid lines
		for (int x = cell_size + 1; x < w; x += cell_size + 1)
		{
			cv::line(mat, cv::Point(x, 0), cv::Point(x, h), cv::Scalar(0x70, 0xee, 0x70));
		}
		// draw horizontal grid lines
		for (int y = cell_size + 1; y < h; y += cell_size + 1)
		{
			cv::line(mat, cv::Point(0, y), cv::Point(w, y), cv::Scalar(0x70, 0xee, 0x70));
		}
	}
	else
	{
		// background is black, so that takes care of all the horizontal and vertical lines that create the grid pattern
		mat = cv::Mat(h, w, CV_8UC3, cv::Scalar(0x00, 0x00, 0x00));

		cv::Point mid_point = content.marks[content.selected_mark].get_corner(corner, content.original_image.size());
		top_left_point = mid_point;
		top_left_point.x -= std::round(cols * 0.5);
		top_left_point.y -= std::round(rows * 0.5);

		// if we have many columns or many rows, then shift the top-left corner a few more pixels to show more of the region-of-interest
		if (cols > 6)
		{
			const int offset = std::floor(cols * 0.25);
			switch(corner)
			{
				case ECorner::kTL:	top_left_point.x += offset;	break;
				case ECorner::kTR:	top_left_point.x -= offset;	break;
				case ECorner::kBR:	top_left_point.x -= offset;	break;
				case ECorner::kBL:	top_left_point.x += offset;	break;
			}
		}

		if (rows > 6)
		{
			const int offset = std::floor(rows * 0.25);
			switch(corner)
			{
				case ECorner::kTL:	top_left_point.y += offset;	break;
				case ECorner::kTR:	top_left_point.y += offset;	break;
				case ECorner::kBR:	top_left_point.y -= offset;	break;
				case ECorner::kBL:	top_left_point.y -= offset;	break;
			}
		}

		// draw all the individual cells
		cv::Mat roi(content.original_image(cv::Rect(top_left_point.x, top_left_point.y, cols, rows)));
		for (int row_index = 0; row_index < rows; row_index ++)
		{
			const uint8_t * src_ptr = roi.ptr(row_index);
			for (int col_index = 0; col_index < cols; col_index ++)
			{
				const uint8_t & red		= src_ptr[col_index * 3 + 2];
				const uint8_t & green	= src_ptr[col_index * 3 + 1];
				const uint8_t & blue	= src_ptr[col_index * 3 + 0];
				const cv::Scalar colour(blue, green, red);
				const cv::Rect r(col_index * (cell_size + 1), row_index * (cell_size + 1), cell_size, cell_size);
				cv::rectangle(mat, r, colour, CV_FILLED, cv::LINE_8);
			}
		}

		const cv::Scalar & colour = content.marks[content.selected_mark].colour;

		// draw the corner cell
		const int x = (mid_point - top_left_point).x * (cell_size + 1);
		const int y = (mid_point - top_left_point).y * (cell_size + 1);
		cv::Rect r(x, y, cell_size, cell_size);
		cv::Mat overlay(cell_size, cell_size, CV_8UC3, colour);
		double alpha = 0.60;
		double beta = 1.0 - alpha;
		cv::addWeighted(overlay, alpha, mat(r), beta, 0, mat(r));

		// draw the larger overlay rectangle
		switch(corner)
		{
			case ECorner::kTL:	r = cv::Rect(x, y, w - x, h - y);					break;
			case ECorner::kTR:	r = cv::Rect(0, y, x + cell_size, h - y);			break;
			case ECorner::kBR:	r = cv::Rect(0, 0, x + cell_size, y + cell_size);	break;
			case ECorner::kBL:	r = cv::Rect(x, 0, w - x, y + cell_size);			break;
		}
		if (r.x >= 0 and r.y >= 0 and r.width > 0 and r.height > 0)
		{
			overlay = cv::Mat(r.height, r.width, CV_8UC3, colour);
			alpha = 0.30;
			beta = 1.0 - alpha;
			cv::addWeighted(overlay, alpha, mat(r), beta, 0, mat(r));
		}
	}

	cached_image = convert_opencv_mat_to_juce_image(mat);
	need_to_rebuild_cache_image = false;

	return;
}


void dm::DMCorner::set_class(size_t class_id)
{
	Log("CLASS=" + std::to_string(class_id));

	return;
}