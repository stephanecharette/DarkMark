/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMCorner::DMCorner() :
	type(EType::kNone),
	cell_size(20),
	cols(10),
	rows(10),
	offset(0, 0),
	poi(0, 0)
{
	mouse_drag_is_enabled = false;

	return;
}


dm::DMCorner::~DMCorner()
{
	return;
}


void dm::DMCorner::set_type(const EType & t)
{
	type = t;

	return;
}


void dm::DMCorner::rebuild_cache_image()
{
	const int h = getHeight();
	const int w = getWidth();

	cv::Mat mat;
	if (original_image.empty())
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
		switch(type)
		{
			case EType::kNone:	offset = cv::Point(0, 0);					break;
			case EType::kTL:	offset = cv::Point(0 - value, 0 - value);	break;
			case EType::kTR:	offset = cv::Point(0 + value, 0 - value);	break;
			case EType::kBR:	offset = cv::Point(0 + value, 0 + value);	break;
			case EType::kBL:	offset = cv::Point(0 - value, 0 + value);	break;
		}

		// background is black, so that takes care of all the horizontal and vertical lines between the cells
		mat = cv::Mat(h, w, CV_8UC3, cv::Scalar(0x00, 0x00, 0x00));

		// draw the individual cells
		cv::Point p = poi + offset;
		cv::Mat roi(original_image(cv::Rect(p.x, p.y, cols, rows)));

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

		const cv::Scalar purple(0xff, 0x00, 0xff);

		// draw the corner cell
		const int x = (cols/2 + offset.x) * (cell_size + 1);
		const int y = (rows/2 + offset.y) * (cell_size + 1);
		cv::Rect r(x, y, cell_size, cell_size);
		cv::Mat overlay(cell_size, cell_size, CV_8UC3, purple);
		double alpha = 0.40;
		double beta = 1.0 - alpha;
		cv::addWeighted(overlay, alpha, mat(r), beta, 0, mat(r));

		// draw the larger overlay rectangle
		switch(type)
		{
			case EType::kTR:
			case EType::kNone:	r = cv::Rect(-1, -1, -1, -1);									break;
			case EType::kTL:	r = cv::Rect(x, y, w - x, h - y);								break;
//			case EType::kTR:	r = cv::Rect(x, y, w - x, h - y);		break;
			case EType::kBR:	r = cv::Rect(-1, -1, -1, -1);			break;
			case EType::kBL:	r = cv::Rect(-1, -1, -1, -1);			break;
		}
		if (r.x >= 0 and r.y >= 0 and r.width > 0 and r.height > 0)
		{
			overlay = cv::Mat(r.width, r.height, CV_8UC3, purple);
			alpha = 0.15;
			beta = 1.0 - alpha;
			cv::addWeighted(overlay, alpha, mat(r), beta, 0, mat(r));
		}
	}

	cached_image = convert_opencv_mat_to_juce_image(mat);
	need_to_rebuild_cache_image = false;

	return;
}


#if 0
void dm::DMCorner::paint(Graphics & g)
{
	CrosshairComponent::paint(g);
	paint_crosshairs(g);


//	g.setOpacity(0.5f);
//	g.fillrect(
#if 0
		// draw the horizontal and vertical lines
		g.setColour(Colour(255, 0, 255));
		int x = (cols/2 + offset.x) * (cell_size + 1) + cell_size / 2;
		int y = (rows/2 + offset.y) * (cell_size + 1) + cell_size / 2;

		switch(type)
		{
			case EType::kNone:
			{
				// do nothing
				break;
			}
			case EType::kTL:
			{
				g.drawVerticalLine(x, y, h);
				g.drawHorizontalLine(y, x, w);
//				g.fillRect(x, y, w, h);
				break;
			}
			case EType::kTR:
			{
				g.drawVerticalLine(x, y, h);
				g.drawHorizontalLine(y, 0, x);
				break;
			}
			case EType::kBR:
			{
				g.drawVerticalLine(x, 0, y);
				g.drawHorizontalLine(y, 0, x);
				break;
			}
			case EType::kBL:
			{
				g.drawVerticalLine(x, 0, y);
				g.drawHorizontalLine(y, x, w);
				break;
			}
		}

		// draw the full corner cell
		x -= cell_size / 2;
		y -= cell_size / 2;
		g.fillRect(x, y, cell_size, cell_size);
	}
#endif

	return;
}
#endif
