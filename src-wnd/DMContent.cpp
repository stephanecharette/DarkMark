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

	return;
}


dm::DMContent::~DMContent(void)
{
	return;
}


void dm::DMContent::resized()
{
	FlexItem::Margin margin;
	margin.bottom = 1.0f;

	const int corner_size = 11 * 16; // each unit is 10x10, but with the border becomes 11x11, multiplied by the number of pixels to show

	FlexBox col(FlexBox::Direction::column, FlexBox::Wrap::noWrap, FlexBox::AlignContent::stretch, FlexBox::AlignItems::stretch, FlexBox::JustifyContent::spaceBetween);
	for (int i = 0; i < 4; i ++)
	{
		col.items.add(FlexItem(corner[i]).withMargin(margin).withMinHeight(corner_size).withMaxHeight(corner_size));
	}

	FlexBox row(FlexBox::Direction::row, FlexBox::Wrap::noWrap, FlexBox::AlignContent::stretch, FlexBox::AlignItems::stretch, FlexBox::JustifyContent::flexStart);
	row.items.add(FlexItem(canvas).withMargin(0).withFlex(1.0));
	row.items.add(FlexItem(col).withMargin(0).withMinWidth(corner_size).withMaxWidth(corner_size));

	auto r = getLocalBounds();
	row.performLayout(r);
}
