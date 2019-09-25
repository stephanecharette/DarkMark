/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMContent::DMContent()
{
	addAndMakeVisible(canvas);
	addAndMakeVisible(corner[0]);
	addAndMakeVisible(corner[1]);
	addAndMakeVisible(corner[2]);
	addAndMakeVisible(corner[3]);

	corner[0].set_type(DMCorner::EType::kTL);
	corner[1].set_type(DMCorner::EType::kTR);
	corner[2].set_type(DMCorner::EType::kBR);
	corner[3].set_type(DMCorner::EType::kBL);

	corner[0].poi = cv::Point(642, 477);
	corner[1].poi = cv::Point(850, 477);
	corner[2].poi = cv::Point(642, 477);
	corner[3].poi = cv::Point(642, 477);

	return;
}


dm::DMContent::~DMContent(void)
{
	return;
}


void dm::DMContent::resized()
{
	const auto r = getLocalBounds();
	const int h = r.getHeight();

	FlexItem::Margin margin;
	margin.bottom = 1.0f;

	// We have 4 "corner" windows to display, and a limited height in which we can do it.  So start off by dividing the height by 4.
	const double quarter = static_cast<double>(h - 6) / 4.0; // -6 because we want a 2-pixel border between each corner window

	// We need to figure out how many cells can fit in this quarter height.  Ideally, we'd like each cell to be around 11 pixels in size.  (Must be an odd number.)
	double number_of_cells = 16.0;
	double cell_size = 0.0;
	while (number_of_cells > 5.0)
	{
		cell_size = std::round((quarter - number_of_cells) / number_of_cells);

		if (cell_size < 11.0)
		{
			number_of_cells --;
			continue;
		}

		break;
	}

	// Now that we know how many cells, and the size of each one, we can calculate the exact size each corner window requires.
	const int corner_size = std::round((cell_size + 1.0) * number_of_cells - 1);

	FlexBox col(FlexBox::Direction::column, FlexBox::Wrap::noWrap, FlexBox::AlignContent::stretch, FlexBox::AlignItems::stretch, FlexBox::JustifyContent::spaceBetween);
	for (int i = 0; i < 4; i ++)
	{
		corner[i].cell_size = cell_size;
		corner[i].cols = number_of_cells;
		corner[i].rows = number_of_cells;
		col.items.add(FlexItem(corner[i]).withMargin(margin).withMinHeight(corner_size).withMaxHeight(corner_size));
	}

	FlexBox row(FlexBox::Direction::row, FlexBox::Wrap::noWrap, FlexBox::AlignContent::stretch, FlexBox::AlignItems::stretch, FlexBox::JustifyContent::flexStart);
	row.items.add(FlexItem(canvas).withMargin(0).withFlex(1.0));
	row.items.add(FlexItem(col).withMargin(0).withMinWidth(corner_size).withMaxWidth(corner_size));

	row.performLayout(r);
}
