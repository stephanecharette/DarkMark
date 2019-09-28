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
	if (original_image.empty() or content.selected_mark < 0)
	{
		// nothing we can do
		mat = cv::Mat(h, w, CV_8UC3, cv::Scalar(0x80, 0xff, 0x80));
	}
	else
	{
		// re-calculate the offsets to bring the central point nearer to the middle as the corner window gets smaller
		const int value = (	cols <= 6 ?	0 :
							cols < 10 ?	std::round(0.10 * cell_size) :
										std::round(0.25 * cell_size) );
		cv::Point offset;
		switch(corner)
		{
			case ECorner::kTL:	offset = cv::Point(0 - value, 0 - value);	break;
			case ECorner::kTR:	offset = cv::Point(0 + value, 0 - value);	break;
			case ECorner::kBR:	offset = cv::Point(0 + value, 0 + value);	break;
			case ECorner::kBL:	offset = cv::Point(0 - value, 0 + value);	break;
		}

		cv::Point poi = content.marks[content.selected_mark].get_corner(corner, original_image.size());

		// background is black, so that takes care of all the horizontal and vertical lines that create the grid pattern
		mat = cv::Mat(h, w, CV_8UC3, cv::Scalar(0x00, 0x00, 0x00));

		// draw the individual cells
		top_left_point = poi + offset;
std::cout << "ROI: x=" << top_left_point.x << " y=" << top_left_point.y << " size=" << cols << "x" << rows << std::endl;
		cv::Mat roi(original_image(cv::Rect(top_left_point.x, top_left_point.y, cols, rows)));

		// draw all the cells
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

		const cv::Scalar colour = content.marks[content.selected_mark].colour;

		// draw the corner cell
		offset = poi - top_left_point;
std::cout << "OFFSET x=" << offset.x << " y=" << offset.y << std::endl;
		const int x = offset.x * (cell_size + 1);
		const int y = offset.y * (cell_size + 1);
std::cout << "rect: x=" << x << " y=" << y << " size=" << cell_size << std::endl;
		cv::Rect r(x, y, cell_size, cell_size);
		cv::Mat overlay(cell_size, cell_size, CV_8UC3, colour);
		double alpha = 0.40;
		double beta = 1.0 - alpha;
		cv::addWeighted(overlay, alpha, mat(r), beta, 0, mat(r));

#if 0
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
			alpha = 0.1;
			beta = 1.0 - alpha;
			cv::addWeighted(overlay, alpha, mat(r), beta, 0, mat(r));
		}
#endif
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
